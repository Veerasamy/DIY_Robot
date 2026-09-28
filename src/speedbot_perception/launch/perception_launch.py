#!/usr/bin/env python3
"""
Launch file for the SpeedBot perception stack.

Launches:
  1. wall_detector  — camera-based wall detection
  2. lidar_processor — LiDAR obstacle detection + Follow-the-Gap

Usage:
  ros2 launch speedbot_perception perception_launch.py
  ros2 launch speedbot_perception perception_launch.py debug:=true
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory('speedbot_perception')
    params_file = os.path.join(pkg_dir, 'config', 'perception_params.yaml')

    return LaunchDescription([
        # ── Launch Arguments ──
        DeclareLaunchArgument(
            'params_file',
            default_value=params_file,
            description='Path to perception parameters YAML file'
        ),
        DeclareLaunchArgument(
            'debug',
            default_value='true',
            description='Enable debug image and marker publishers'
        ),

        # ── Wall Detector Node ──
        Node(
            package='speedbot_perception',
            executable='wall_detector',
            name='wall_detector',
            output='screen',
            parameters=[LaunchConfiguration('params_file')],
            remappings=[],
        ),

        # ── LiDAR Processor Node ──
        Node(
            package='speedbot_perception',
            executable='lidar_processor',
            name='lidar_processor',
            output='screen',
            parameters=[LaunchConfiguration('params_file')],
            remappings=[],
        ),
    ])
