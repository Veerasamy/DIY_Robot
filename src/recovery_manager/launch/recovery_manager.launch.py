"""Bring up recovery_manager. Requires safety_manager, obstacle_detection
(for /obstacle/costmap) and sensor_fusion to already be running; lane_detection
is optional (used only to reorient onto the centerline during RECOVER)."""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    node = Node(
        package="recovery_manager",
        executable="recovery_manager_node",
        name="recovery_manager",
        output="screen",
        parameters=[{
            "wheelbase_m": 0.045,
            "max_steering_angle_rad": 0.2812,
            "reverse_speed_mps": 0.5,
            "recover_creep_speed_mps": 0.4,
            "max_accel_mps2": 1.0,
            "max_yaw_rate_rad_s": 1.5,
            "max_yaw_accel_rad_s2": 2.5,
            "vehicle_radius_m": 0.3,
            "collision_margin_m": 0.1,
            "reverse_min_duration_sec": 1.0,
            "reverse_clear_distance_m": 1.0,
            "recovered_heading_tol_rad": 0.15,
            "control_rate_hz": 20.0,
        }],
    )
    return LaunchDescription([node])
