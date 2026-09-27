"""Bring up the lane_detection node. Requires zed_interface to already be publishing
/zed/rgb/image_raw."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    debug_arg = DeclareLaunchArgument(
        "publish_debug_image", default_value="false",
        description="Publish /lane/debug_image (birds-eye overlay) for rviz/tuning")
    rgb_topic_arg = DeclareLaunchArgument(
        "rgb_image_topic", default_value="/zed/rgb/image_raw",
        description="Remap source for the RGB image -- point this at the real "
                     "zed-ros2-wrapper's topic (e.g. /zed2i/zed_node/rgb/color/rect/image) "
                     "when not using our own zed_interface node")

    node = Node(
        package="lane_detection",
        executable="lane_detection_node",
        name="lane_detection",
        output="screen",
        parameters=[{
            "publish_debug_image": LaunchConfiguration("publish_debug_image"),
        }],
        remappings=[("/zed/rgb/image_raw", LaunchConfiguration("rgb_image_topic"))],
    )

    return LaunchDescription([debug_arg, rgb_topic_arg, node])
