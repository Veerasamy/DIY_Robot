#!/usr/bin/env python3
"""
Launch file for the SpeedBot control stack.

Launches:
  1. drive_controller — fuses perception → steering + speed
  2. motor_bridge     — converts commands → PWM over serial

Usage:
  ros2 launch speedbot_control control_launch.py
  ros2 launch speedbot_control control_launch.py dry_run:=true
"""

import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_dir = get_package_share_directory('speedbot_control')
    params_file = os.path.join(pkg_dir, 'config', 'control_params.yaml')

    return LaunchDescription([
        # ── Launch Arguments ──
        DeclareLaunchArgument(
            'params_file',
            default_value=params_file,
            description='Path to control parameters YAML file'
        ),

        # ── Drive Controller Node ──
        Node(
            package='speedbot_control',
            executable='drive_controller',
            name='drive_controller',
            output='screen',
            parameters=[LaunchConfiguration('params_file')],
        ),

        # ── Motor Bridge Node ──
        Node(
            package='speedbot_control',
            executable='motor_bridge',
            name='motor_bridge',
            output='screen',
            parameters=[LaunchConfiguration('params_file')],
        ),
    ])
