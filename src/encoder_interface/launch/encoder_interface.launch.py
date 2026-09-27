"""Bring up the wheel-encoder GPIO quadrature reader. Requires libgpiod-dev
at build time to actually read hardware — see package README."""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    node = Node(
        package="encoder_interface",
        executable="encoder_node",
        name="encoder_interface",
        output="screen",
        parameters=[{
            "gpio_chip": "/dev/gpiochip0",
            # GPIO offsets — MEASURE/CONFIRM against your wiring with
            # `gpioinfo`, these are placeholders.
            "gpio_pin_a": 0,
            "gpio_pin_b": 0,
            "pulses_per_revolution": 40.0,
            "wheel_circumference_m": 0.2,
            "velocity_lowpass_alpha": 0.3,
            "publish_rate_hz": 50.0,
        }],
    )
    return LaunchDescription([node])
