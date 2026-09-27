"""Bring up the global raceline loader + local/global arbitration node.
Requires sensor_fusion (/ekf/odom) and lane_detection (/lane/centerline) to
already be running for full functionality; degrades to raceline-only or
centerline-only if either is missing."""
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    mode_arg = DeclareLaunchArgument(
        "mode", default_value="speed", description="'speed' or 'obstacle'")

    default_raceline = os.path.join(
        get_package_share_directory("planner"), "config", "raceline.yaml")

    node = Node(
        package="planner",
        executable="local_planner_node",
        name="planner",
        output="screen",
        parameters=[{
            "raceline_yaml_path": default_raceline,
            "race_mode": LaunchConfiguration("mode"),
            "lookahead_window_m": 15.0,
            "blend_weight": 0.3,
            "control_rate_hz": 20.0,
        }],
    )
    return LaunchDescription([mode_arg, node])
