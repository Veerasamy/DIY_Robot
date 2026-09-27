"""Bring up the ZED 2i interface node."""
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.actions import Node


def generate_launch_description():
    mode_arg = DeclareLaunchArgument(
        "mode", default_value="speed",
        description="'speed' or 'obstacle' — selects config/speed_mode.yaml or config/obstacle_mode.yaml")

    config_file = PathJoinSubstitution([
        get_package_share_directory("zed_interface"),
        "config",
        [LaunchConfiguration("mode"), "_mode.yaml"],
    ])

    node = Node(
        package="zed_interface",
        executable="zed_interface_node",
        name="zed_interface",
        output="screen",
        parameters=[config_file],
    )

    return LaunchDescription([mode_arg, node])
