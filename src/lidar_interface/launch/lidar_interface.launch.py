"""Bring up the vendored rplidar_ros driver plus this package's scan-cleanup
filter node. The driver's raw output is remapped to /lidar/scan_raw so the
filter node (not the driver) owns the canonical /scan topic obstacle_detection
subscribes to.

NOTE: rplidar_ros's exact package/executable name and parameter set vary
across releases (community-maintained, not part of this repo). Confirm
`ros2 pkg executables rplidar_ros` on your installed version and adjust the
Node action below if it differs from `rplidar_node`."""
import os

from ament_index_python.packages import PackageNotFoundError, get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory("lidar_interface"), "config", "rplidar.yaml")

    actions = []
    # rplidar_ros is an external, vendored driver (not part of this
    # workspace) -- if it isn't installed, skip it rather than aborting the
    # whole bringup; scan_filter_node will just have no input until it's
    # installed (see package README).
    try:
        get_package_share_directory("rplidar_ros")
        actions.append(Node(
            package="rplidar_ros",
            executable="rplidar_node",
            name="rplidar_node",
            output="screen",
            parameters=[config],
            remappings=[("scan", "/lidar/scan_raw")],
        ))
    except PackageNotFoundError:
        print(
            "[lidar_interface] WARNING: rplidar_ros not installed, skipping the "
            "driver node -- install it per this package's README to get real "
            "LiDAR data on /lidar/scan_raw.")

    actions.append(Node(
        package="lidar_interface",
        executable="scan_filter_node",
        name="lidar_interface",
        output="screen",
        parameters=[{
            "input_topic": "/lidar/scan_raw",
            "output_topic": "/scan",
            "range_min_m": 0.15,
            "range_max_m": 12.0,
            # Mounting-bracket occlusion sector — MEASURE on the assembled
            # SCRT10; 0.0/0.0 (equal) disables blind-spot masking.
            "blind_spot_start_rad": 0.0,
            "blind_spot_end_rad": 0.0,
            "frame_id_override": "",
        }],
    ))

    return LaunchDescription(actions)
