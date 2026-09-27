"""Bring up the VFH obstacle avoidance node. Requires zed_interface (depth) and
a LiDAR driver publishing /scan (e.g. rplidar_ros) to be running."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    depth_topic_arg = DeclareLaunchArgument(
        "depth_image_topic", default_value="/zed/depth/image",
        description="Remap source for the depth image -- point this at the real "
                     "zed-ros2-wrapper's topic (e.g. /zed2i/zed_node/depth/depth_registered) "
                     "when not using our own zed_interface node")

    node = Node(
        package="obstacle_detection",
        executable="obstacle_avoidance_node",
        name="obstacle_avoidance",
        output="screen",
        parameters=[{
            "fov_half_angle_rad": 1.5708,
            "sector_resolution_rad": 0.0349,
            "smoothing_kernel_width": 5,
            "max_range_m": 6.0,
            "obstacle_threshold": 1.5,
            "vehicle_half_width_m": 0.1365,
            "safety_margin_m": 0.15,
            "camera_mount_height_m": 0.195,
            "camera_tilt_rad": 0.0,
            "ground_clearance_min_m": 0.03,
            "obstacle_height_max_m": 0.25,
            "braking_decel_mps2": 6.0,
            "braking_margin_m": 0.3,
            "control_rate_hz": 20.0,
        }],
        remappings=[("/zed/depth/image", LaunchConfiguration("depth_image_topic"))],
    )
    return LaunchDescription([depth_topic_arg, node])
