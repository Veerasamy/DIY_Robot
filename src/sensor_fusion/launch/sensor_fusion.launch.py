"""Bring up the sensor_fusion EKF node. Requires zed_interface for VO/IMU;
encoder_interface (future) for /encoder/velocity_mps is optional -- the EKF
degrades gracefully to VO+IMU-only fusion if it's absent."""
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    vo_topic_arg = DeclareLaunchArgument(
        "vo_odom_topic", default_value="/zed/odom/vo",
        description="Remap source for visual odometry -- point this at the real "
                     "zed-ros2-wrapper's topic (e.g. /zed2i/zed_node/odom) when not "
                     "using our own zed_interface node")
    imu_topic_arg = DeclareLaunchArgument(
        "imu_topic", default_value="/zed/imu/data",
        description="Remap source for IMU data -- point this at the real "
                     "zed-ros2-wrapper's topic (e.g. /zed2i/zed_node/imu/data) when not "
                     "using our own zed_interface node")

    node = Node(
        package="sensor_fusion",
        executable="ekf_node",
        name="sensor_fusion_ekf",
        output="screen",
        parameters=[{
            "process_noise_pos": 0.01,
            "process_noise_theta": 0.005,
            "process_noise_v": 0.2,
            "process_noise_omega": 0.1,
            "encoder_variance": 0.01,
            "imu_yaw_rate_variance": 0.0009,
        }],
        remappings=[
            ("/zed/odom/vo", LaunchConfiguration("vo_odom_topic")),
            ("/zed/imu/data", LaunchConfiguration("imu_topic")),
        ],
    )
    return LaunchDescription([vo_topic_arg, imu_topic_arg, node])
