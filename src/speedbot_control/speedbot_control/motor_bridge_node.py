#!/usr/bin/env python3
"""
Motor Bridge Node — SpeedBot Control
======================================
Translates high-level Ackermann drive commands (Twist) into low-level
PWM signals sent to the motor controller via serial (e.g., Arduino, VESC).

This is the hardware abstraction layer — modify the serial protocol
to match your specific motor controller.

Protocol (default — customize for your hardware):
  Sends ASCII commands over serial:
    "S<pwm>\n"  — steering servo PWM (1000-2000 μs)
    "T<pwm>\n"  — throttle ESC PWM (1000-2000 μs)

Subscribes:
  - /control/ackermann_cmd (Twist)
    linear.x  = speed (m/s)
    angular.z = steering angle (rad)

Publishes:
  - /control/motor_status (String) — JSON status of motor bridge
"""

import math
import serial
import time

import rclpy
from rclpy.node import Node

from geometry_msgs.msg import Twist
from std_msgs.msg import String


class MotorBridgeNode(Node):
    """
    Bridges ROS 2 Twist commands to hardware PWM signals via serial.
    Modify the _send_to_hardware() method for your specific motor controller.
    """

    def __init__(self):
        super().__init__('motor_bridge')

        # ── Declare Parameters ──
        self.declare_parameter('cmd_topic', '/control/ackermann_cmd')
        self.declare_parameter('serial_port', '/dev/ttyUSB1')
        self.declare_parameter('baud_rate', 115200)

        self.declare_parameter('steering_pwm_center', 1500)
        self.declare_parameter('steering_pwm_range', 500)
        self.declare_parameter('steering_invert', False)

        self.declare_parameter('throttle_pwm_neutral', 1500)
        self.declare_parameter('throttle_pwm_max', 2000)
        self.declare_parameter('throttle_pwm_min', 1000)
        self.declare_parameter('throttle_invert', False)

        self.declare_parameter('command_rate_hz', 50.0)

        # Max steering angle must match the drive controller
        self.declare_parameter('max_steering_angle_deg', 30.0)
        # Max speed must match the drive controller
        self.declare_parameter('max_speed_mps', 3.0)

        # ── Load Parameters ──
        self.steer_center = self.get_parameter('steering_pwm_center').value
        self.steer_range = self.get_parameter('steering_pwm_range').value
        self.steer_invert = self.get_parameter('steering_invert').value

        self.throttle_neutral = self.get_parameter('throttle_pwm_neutral').value
        self.throttle_max = self.get_parameter('throttle_pwm_max').value
        self.throttle_min = self.get_parameter('throttle_pwm_min').value
        self.throttle_invert = self.get_parameter('throttle_invert').value

        self.max_steer_rad = math.radians(
            self.get_parameter('max_steering_angle_deg').value
        )
        self.max_speed = self.get_parameter('max_speed_mps').value

        cmd_rate = self.get_parameter('command_rate_hz').value

        # ── Latest Command ──
        self._latest_steer_pwm = self.steer_center
        self._latest_throttle_pwm = self.throttle_neutral
        self._last_cmd_time = 0.0

        # ── Serial Connection ──
        serial_port = self.get_parameter('serial_port').value
        baud_rate = self.get_parameter('baud_rate').value
        self.serial_conn = None

        try:
            self.serial_conn = serial.Serial(serial_port, baud_rate, timeout=0.1)
            time.sleep(2.0)  # Wait for Arduino/MCU to reset after serial connect
            self.get_logger().info(f'Serial connected: {serial_port} @ {baud_rate}')
        except serial.SerialException as e:
            self.get_logger().error(
                f'Failed to open serial port {serial_port}: {e}\n'
                f'Motor bridge running in DRY RUN mode (no hardware commands sent)'
            )

        # ── Subscriber ──
        cmd_topic = self.get_parameter('cmd_topic').value
        self.create_subscription(Twist, cmd_topic, self._cmd_callback, 10)

        # ── Publisher ──
        self.pub_status = self.create_publisher(String, '/control/motor_status', 10)

        # ── Command Timer ──
        self.create_timer(1.0 / cmd_rate, self._send_command)

        # ── Safety Timer — stop if no commands received for 500ms ──
        self.create_timer(0.1, self._check_timeout)

        self.get_logger().info(
            f'MotorBridge started | Steer: {self.steer_center}±{self.steer_range}μs | '
            f'Throttle: [{self.throttle_min}, {self.throttle_max}]μs | '
            f'Rate: {cmd_rate} Hz'
        )

    def _cmd_callback(self, msg: Twist):
        """Convert Twist command to PWM values."""
        self._last_cmd_time = time.time()

        speed = msg.linear.x       # m/s
        steer_rad = msg.angular.z  # radians

        # ── Steering → PWM ──
        # Normalize steering angle to [-1, 1]
        steer_norm = steer_rad / self.max_steer_rad if self.max_steer_rad > 0 else 0.0
        steer_norm = max(-1.0, min(1.0, steer_norm))

        if self.steer_invert:
            steer_norm = -steer_norm

        steer_pwm = int(self.steer_center + steer_norm * self.steer_range)
        steer_pwm = max(self.steer_center - self.steer_range,
                        min(self.steer_center + self.steer_range, steer_pwm))

        # ── Speed → PWM ──
        # Normalize speed to [0, 1] for forward, [-1, 0] for reverse
        speed_norm = speed / self.max_speed if self.max_speed > 0 else 0.0
        speed_norm = max(-1.0, min(1.0, speed_norm))

        if self.throttle_invert:
            speed_norm = -speed_norm

        if speed_norm >= 0:
            throttle_pwm = int(
                self.throttle_neutral +
                speed_norm * (self.throttle_max - self.throttle_neutral)
            )
        else:
            throttle_pwm = int(
                self.throttle_neutral +
                speed_norm * (self.throttle_neutral - self.throttle_min)
            )

        throttle_pwm = max(self.throttle_min, min(self.throttle_max, throttle_pwm))

        self._latest_steer_pwm = steer_pwm
        self._latest_throttle_pwm = throttle_pwm

    def _send_command(self):
        """Send the latest PWM values to hardware at command_rate_hz."""
        self._send_to_hardware(self._latest_steer_pwm, self._latest_throttle_pwm)

        # Publish status
        status = String()
        status.data = (
            f'{{"steer_pwm": {self._latest_steer_pwm}, '
            f'"throttle_pwm": {self._latest_throttle_pwm}, '
            f'"serial_ok": {str(self.serial_conn is not None).lower()}}}'
        )
        self.pub_status.publish(status)

    def _send_to_hardware(self, steer_pwm: int, throttle_pwm: int):
        """
        Send PWM commands to ESP32 over USB serial.

        Protocol matches speedbot_esp32.ino firmware:
          Send:    "S<steer_pwm>,T<throttle_pwm>\n"
          Receive: "OK S<steer> T<throttle>\n"
        """
        if self.serial_conn is None:
            return

        try:
            cmd = f'S{steer_pwm},T{throttle_pwm}\n'
            self.serial_conn.write(cmd.encode('ascii'))

            # Read any available response (non-blocking)
            if self.serial_conn.in_waiting > 0:
                response = self.serial_conn.readline().decode('ascii', errors='ignore').strip()
                if response and not response.startswith('OK'):
                    self.get_logger().warn(f'ESP32: {response}')

        except serial.SerialException as e:
            self.get_logger().error(f'Serial write failed: {e}')
            self.serial_conn = None

    def _check_timeout(self):
        """Safety: send neutral commands if no Twist received recently."""
        if time.time() - self._last_cmd_time > 0.5 and self._last_cmd_time > 0:
            self._latest_steer_pwm = self.steer_center
            self._latest_throttle_pwm = self.throttle_neutral

    def destroy_node(self):
        """Send STOP command to ESP32 and close serial on shutdown."""
        if self.serial_conn is not None:
            try:
                self.serial_conn.write(b'STOP\n')
                time.sleep(0.1)
                self.serial_conn.close()
                self.get_logger().info('ESP32: STOP sent — serial closed')
            except Exception:
                pass
        super().destroy_node()



def main(args=None):
    rclpy.init(args=args)
    node = MotorBridgeNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
