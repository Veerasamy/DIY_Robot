"""Bring up the safety_manager E-Stop bridge. Always launch this first."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    serial_device_arg = DeclareLaunchArgument(
        "serial_device", default_value="/dev/ttyUSB_estop",
        description="udev-mapped serial device for the ESP32 E-Stop bridge")
    serial_baud_arg = DeclareLaunchArgument("serial_baud", default_value="115200")
    link_timeout_arg = DeclareLaunchArgument(
        "link_timeout_sec", default_value="0.3",
        description="Max age of a valid UART packet before treating link as lost "
                     "(must stay well under the 1.0s competition stop budget)")
    bypass_estop_hardware_arg = DeclareLaunchArgument(
        "bypass_estop_hardware", default_value="false",
        description="Bench-test only: if true, a missing/lost E-Stop UART link is NOT "
                     "treated as a press. NEVER set true for an actual driving run.")

    node = Node(
        package="safety_manager",
        executable="estop_bridge_node",
        name="safety_manager",
        output="screen",
        parameters=[{
            "serial_device": LaunchConfiguration("serial_device"),
            "serial_baud": LaunchConfiguration("serial_baud"),
            "link_timeout_sec": LaunchConfiguration("link_timeout_sec"),
            "bypass_estop_hardware": LaunchConfiguration("bypass_estop_hardware"),
        }],
    )

    return LaunchDescription([
        serial_device_arg,
        serial_baud_arg,
        link_timeout_arg,
        bypass_estop_hardware_arg,
        node,
    ])
