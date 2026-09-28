#!/usr/bin/env python3
"""
Drive Controller Node — SpeedBot Control
==========================================
Main autonomy controller that fuses perception outputs into Ackermann
steering + throttle commands.

Pipeline:
  Camera lane offset  ──┐
                         ├──► Fusion ──► Steering PID ──► Ackermann Cmd
  LiDAR gap angle     ──┘                    │
                                             ▼
  Closest obstacle  ──────────────────► Speed Controller
  Track boundaries  ──────────────────► Curvature Estimation

Subscribes:
  - /perception/lane_center_offset   (Float64)
  - /perception/best_gap_angle       (Float64)
  - /perception/closest_obstacle     (Float64)
  - /perception/track_boundaries     (Float64MultiArray)

Publishes:
  - /control/ackermann_cmd           (AckermannCmd — custom msg as Twist)
  - /control/debug_info              (String — JSON debug telemetry)
"""

import math
import time
import json

import rclpy
from rclpy.node import Node

from std_msgs.msg import Float64, Float64MultiArray, String
from geometry_msgs.msg import Twist


class PIDController:
    """Discrete PID controller with anti-windup and derivative filtering."""

    def __init__(self, kp=1.0, ki=0.0, kd=0.0,
                 integral_max=1.0, output_min=-1.0, output_max=1.0):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.integral_max = integral_max
        self.output_min = output_min
        self.output_max = output_max

        self._integral = 0.0
        self._prev_error = 0.0
        self._prev_time = None

    def reset(self):
        """Reset the PID state."""
        self._integral = 0.0
        self._prev_error = 0.0
        self._prev_time = None

    def compute(self, error: float, current_time: float) -> float:
        """
        Compute PID output given the current error.

        Args:
            error: The current error (setpoint - measurement).
            current_time: Current timestamp in seconds.

        Returns:
            PID output clamped to [output_min, output_max].
        """
        if self._prev_time is None:
            self._prev_time = current_time
            self._prev_error = error
            return self.kp * error

        dt = current_time - self._prev_time
        if dt <= 0.0:
            return self.kp * error

        # Proportional
        p_term = self.kp * error

        # Integral with anti-windup
        self._integral += error * dt
        self._integral = max(-self.integral_max,
                             min(self.integral_max, self._integral))
        i_term = self.ki * self._integral

        # Derivative (on error, with basic filtering)
        d_error = (error - self._prev_error) / dt
        d_term = self.kd * d_error

        self._prev_error = error
        self._prev_time = current_time

        # Sum and clamp
        output = p_term + i_term + d_term
        output = max(self.output_min, min(self.output_max, output))

        return output


class DriveControllerNode(Node):
    """
    Fuses camera and LiDAR perception to produce Ackermann drive commands.

    Steering: PID on lane center offset, blended with LiDAR gap angle.
    Speed: Curvature-based profiling with obstacle braking.
    Safety: E-stop on perception timeout or imminent collision.
    """

    def __init__(self):
        super().__init__('drive_controller')

        # ── Declare All Parameters ──
        self._declare_params()

        # ── Load Parameters ──
        self.control_mode = self.get_parameter('control_mode').value
        self.camera_weight = self.get_parameter('camera_weight').value
        self.lidar_weight = self.get_parameter('lidar_weight').value

        self.max_steer_rad = math.radians(
            self.get_parameter('max_steering_angle_deg').value
        )
        self.wheelbase = self.get_parameter('wheelbase_m').value

        self.max_speed = self.get_parameter('speed.max_speed_mps').value
        self.min_speed = self.get_parameter('speed.min_speed_mps').value
        self.cruise_speed = self.get_parameter('speed.cruise_speed_mps').value
        self.slowdown_factor = self.get_parameter('speed.slowdown_factor').value
        self.brake_dist = self.get_parameter('speed.obstacle_brake_distance').value
        self.stop_dist = self.get_parameter('speed.obstacle_stop_distance').value

        self.perception_timeout = self.get_parameter('safety.perception_timeout_sec').value
        self.estop_enabled = self.get_parameter('safety.enable_estop').value

        control_rate = self.get_parameter('control_rate_hz').value

        # ── Steering PID ──
        pid_params = {
            'kp': self.get_parameter('steering_pid.kp').value,
            'ki': self.get_parameter('steering_pid.ki').value,
            'kd': self.get_parameter('steering_pid.kd').value,
            'integral_max': self.get_parameter('steering_pid.integral_max').value,
            'output_min': self.get_parameter('steering_pid.output_min').value,
            'output_max': self.get_parameter('steering_pid.output_max').value,
        }
        self.steering_pid = PIDController(**pid_params)

        # ── Perception State ──
        self._lane_offset = 0.0          # -1 (left) to +1 (right)
        self._gap_angle = 0.0            # radians
        self._closest_obstacle = 999.0   # meters
        self._left_wall_dist = -1.0      # meters (-1 = unknown)
        self._right_wall_dist = -1.0     # meters (-1 = unknown)

        self._last_lane_time = 0.0
        self._last_gap_time = 0.0
        self._last_obstacle_time = 0.0

        # ── Robot State ──
        self._is_estopped = False
        self._current_speed_cmd = 0.0
        self._current_steer_cmd = 0.0

        # ── Subscribers ──
        lane_topic = self.get_parameter('lane_offset_topic').value
        gap_topic = self.get_parameter('gap_angle_topic').value
        obstacle_topic = self.get_parameter('closest_obstacle_topic').value
        boundary_topic = self.get_parameter('boundary_topic').value

        self.create_subscription(Float64, lane_topic, self._lane_cb, 10)
        self.create_subscription(Float64, gap_topic, self._gap_cb, 10)
        self.create_subscription(Float64, obstacle_topic, self._obstacle_cb, 10)
        self.create_subscription(Float64MultiArray, boundary_topic, self._boundary_cb, 10)

        # ── Publishers ──
        # Using Twist as a portable Ackermann command:
        #   twist.linear.x  = speed (m/s)
        #   twist.angular.z = steering angle (rad, + = left, - = right)
        cmd_topic = self.get_parameter('cmd_topic').value
        self.pub_cmd = self.create_publisher(Twist, cmd_topic, 10)
        self.pub_debug = self.create_publisher(String, '/control/debug_info', 10)

        # ── Control Timer ──
        self.create_timer(1.0 / control_rate, self._control_loop)

        self.get_logger().info(
            f'DriveController started | Mode: {self.control_mode} | '
            f'Weights: cam={self.camera_weight} lid={self.lidar_weight} | '
            f'Max steer: {math.degrees(self.max_steer_rad):.0f}° | '
            f'Speed: [{self.min_speed}, {self.max_speed}] m/s | '
            f'Rate: {control_rate} Hz'
        )

    def _declare_params(self):
        """Declare all ROS 2 parameters with defaults."""
        self.declare_parameter('lane_offset_topic', '/perception/lane_center_offset')
        self.declare_parameter('gap_angle_topic', '/perception/best_gap_angle')
        self.declare_parameter('closest_obstacle_topic', '/perception/closest_obstacle')
        self.declare_parameter('boundary_topic', '/perception/track_boundaries')
        self.declare_parameter('cmd_topic', '/control/ackermann_cmd')

        self.declare_parameter('control_mode', 'fused')
        self.declare_parameter('camera_weight', 0.65)
        self.declare_parameter('lidar_weight', 0.35)

        self.declare_parameter('steering_pid.kp', 0.8)
        self.declare_parameter('steering_pid.ki', 0.01)
        self.declare_parameter('steering_pid.kd', 0.15)
        self.declare_parameter('steering_pid.integral_max', 0.5)
        self.declare_parameter('steering_pid.output_min', -1.0)
        self.declare_parameter('steering_pid.output_max', 1.0)

        self.declare_parameter('max_steering_angle_deg', 30.0)
        self.declare_parameter('wheelbase_m', 0.35)

        self.declare_parameter('speed.max_speed_mps', 3.0)
        self.declare_parameter('speed.min_speed_mps', 0.8)
        self.declare_parameter('speed.cruise_speed_mps', 1.5)
        self.declare_parameter('speed.slowdown_factor', 3.0)
        self.declare_parameter('speed.obstacle_brake_distance', 1.5)
        self.declare_parameter('speed.obstacle_stop_distance', 0.4)

        self.declare_parameter('safety.perception_timeout_sec', 0.5)
        self.declare_parameter('safety.enable_estop', True)

        self.declare_parameter('control_rate_hz', 30.0)

    # ──────────────────────────────────────────────
    #  Perception Callbacks
    # ──────────────────────────────────────────────
    def _lane_cb(self, msg: Float64):
        self._lane_offset = msg.data
        self._last_lane_time = time.time()

    def _gap_cb(self, msg: Float64):
        self._gap_angle = msg.data
        self._last_gap_time = time.time()

    def _obstacle_cb(self, msg: Float64):
        self._closest_obstacle = msg.data
        self._last_obstacle_time = time.time()

    def _boundary_cb(self, msg: Float64MultiArray):
        if len(msg.data) >= 2:
            self._left_wall_dist = msg.data[0]
            self._right_wall_dist = msg.data[1]

    # ──────────────────────────────────────────────
    #  Main Control Loop
    # ──────────────────────────────────────────────
    def _control_loop(self):
        """Run at control_rate_hz — compute steering + speed, publish cmd."""
        now = time.time()
        cmd = Twist()

        # ── Safety Check: Perception Timeout ──
        if self.estop_enabled:
            lane_stale = (now - self._last_lane_time) > self.perception_timeout
            gap_stale = (now - self._last_gap_time) > self.perception_timeout

            if self.control_mode == 'camera' and lane_stale:
                self._emergency_stop(cmd, 'Camera perception timeout')
                return
            elif self.control_mode == 'lidar' and gap_stale:
                self._emergency_stop(cmd, 'LiDAR perception timeout')
                return
            elif self.control_mode == 'fused' and lane_stale and gap_stale:
                self._emergency_stop(cmd, 'All perception timeout')
                return

        # ── Safety Check: Imminent Collision ──
        if self._closest_obstacle < self.stop_dist:
            self._emergency_stop(cmd, f'Obstacle at {self._closest_obstacle:.2f}m')
            return

        # Clear e-stop if conditions are resolved
        if self._is_estopped:
            self._is_estopped = False
            self.steering_pid.reset()
            self.get_logger().info('E-stop cleared — resuming control')

        # ── Compute Steering ──
        steering_rad = self._compute_steering(now)

        # ── Compute Speed ──
        speed_mps = self._compute_speed(steering_rad)

        # ── Build Command ──
        cmd.linear.x = speed_mps
        cmd.angular.z = steering_rad

        self._current_speed_cmd = speed_mps
        self._current_steer_cmd = steering_rad

        self.pub_cmd.publish(cmd)

        # ── Debug Telemetry ──
        self._publish_debug(steering_rad, speed_mps, now)

    def _compute_steering(self, now: float) -> float:
        """
        Compute steering angle in radians based on control mode.

        Camera: PID on lane center offset (error = offset, target = 0)
        LiDAR:  Directly use gap angle
        Fused:  Weighted blend of both
        """
        camera_steer = 0.0
        lidar_steer = 0.0

        # Camera-based steering (PID on lane offset)
        # Offset > 0 means robot is right of center → steer left (positive)
        # We negate because we want to correct toward center
        camera_error = -self._lane_offset
        camera_steer = self.steering_pid.compute(camera_error, now)
        # Convert normalized [-1, 1] → radians
        camera_steer_rad = camera_steer * self.max_steer_rad

        # LiDAR-based steering (Follow-the-Gap angle)
        # Gap angle is already in radians, positive = left
        lidar_steer_rad = max(-self.max_steer_rad,
                              min(self.max_steer_rad, self._gap_angle))

        # ── Select Based on Mode ──
        if self.control_mode == 'camera':
            steering_rad = camera_steer_rad

        elif self.control_mode == 'lidar':
            steering_rad = lidar_steer_rad

        else:  # 'fused'
            steering_rad = (
                self.camera_weight * camera_steer_rad +
                self.lidar_weight * lidar_steer_rad
            )

        # ── Wall-Proximity Correction ──
        # If one wall is much closer than the other, add a correction bias
        steering_rad = self._apply_wall_proximity_correction(steering_rad)

        # Clamp to physical limits
        steering_rad = max(-self.max_steer_rad,
                           min(self.max_steer_rad, steering_rad))

        return steering_rad

    def _apply_wall_proximity_correction(self, steering_rad: float) -> float:
        """
        If one wall is significantly closer than the other, add a small
        corrective bias to steer away from it. Acts as a safety net.
        """
        left_d = self._left_wall_dist
        right_d = self._right_wall_dist

        # Only apply if both walls are detected
        if left_d < 0 or right_d < 0:
            return steering_rad

        total = left_d + right_d
        if total < 0.01:
            return steering_rad

        # Imbalance: positive = closer to right wall → steer left
        imbalance = (right_d - left_d) / total  # range: [-1, 1]

        # Apply a gentle correction (scaled by proximity)
        min_wall = min(left_d, right_d)
        proximity_factor = max(0.0, 1.0 - min_wall / 0.5)  # ramps up below 0.5m

        correction = imbalance * proximity_factor * 0.15 * self.max_steer_rad
        return steering_rad + correction

    def _compute_speed(self, steering_rad: float) -> float:
        """
        Compute target speed based on:
        1. Steering angle (curvature) — slow in curves
        2. Obstacle proximity — brake when close
        """
        # ── Curvature-Based Speed ──
        # Higher steering angle = tighter curve = slower speed
        abs_steer = abs(steering_rad)
        curvature_speed = self.max_speed - self.slowdown_factor * abs_steer
        curvature_speed = max(self.min_speed, min(self.max_speed, curvature_speed))

        # ── Obstacle-Based Braking ──
        obstacle_d = self._closest_obstacle
        if obstacle_d < self.brake_dist:
            # Linear ramp from full speed at brake_dist to 0 at stop_dist
            brake_range = self.brake_dist - self.stop_dist
            if brake_range > 0:
                brake_factor = (obstacle_d - self.stop_dist) / brake_range
                brake_factor = max(0.0, min(1.0, brake_factor))
            else:
                brake_factor = 0.0
            obstacle_speed = curvature_speed * brake_factor
        else:
            obstacle_speed = curvature_speed

        return max(0.0, obstacle_speed)

    def _emergency_stop(self, cmd: Twist, reason: str):
        """Publish zero-velocity command and log reason."""
        cmd.linear.x = 0.0
        cmd.angular.z = 0.0
        self.pub_cmd.publish(cmd)

        if not self._is_estopped:
            self._is_estopped = True
            self.get_logger().warn(f'⛔ E-STOP: {reason}')

    # ──────────────────────────────────────────────
    #  Debug Telemetry
    # ──────────────────────────────────────────────
    def _publish_debug(self, steer_rad: float, speed: float, now: float):
        """Publish JSON telemetry for debugging and logging."""
        debug = {
            'timestamp': now,
            'mode': self.control_mode,
            'steering_deg': round(math.degrees(steer_rad), 2),
            'speed_mps': round(speed, 3),
            'lane_offset': round(self._lane_offset, 4),
            'gap_angle_deg': round(math.degrees(self._gap_angle), 2),
            'closest_obstacle_m': round(self._closest_obstacle, 3),
            'left_wall_m': round(self._left_wall_dist, 3),
            'right_wall_m': round(self._right_wall_dist, 3),
            'estopped': self._is_estopped,
        }

        msg = String()
        msg.data = json.dumps(debug)
        self.pub_debug.publish(msg)


def main(args=None):
    rclpy.init(args=args)
    node = DriveControllerNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        # Send stop command on shutdown
        stop_cmd = Twist()
        node.pub_cmd.publish(stop_cmd)
        node.get_logger().info('Shutdown — sent stop command')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
