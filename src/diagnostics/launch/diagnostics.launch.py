"""Bring up the system_health diagnostics node."""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    node = Node(
        package="diagnostics",
        executable="system_health_node",
        name="system_health",
        output="screen",
        parameters=[{
            "stale_after_sec": 1.0,
            "publish_rate_hz": 2.0,
            "gpu_cpu_sample_rate_hz": 0.5,
        }],
    )
    return LaunchDescription([node])
