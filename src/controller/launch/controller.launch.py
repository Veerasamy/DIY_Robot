"""Bring up the Pure Pursuit controller. Requires lane_detection, sensor_fusion
and safety_manager to already be running."""
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    node = Node(
        package="controller",
        executable="pure_pursuit_node",
        name="pure_pursuit_controller",
        output="screen",
        parameters=[{
            "controller_type": "pure_pursuit",  # pure_pursuit | stanley | mpc
            # measured: 4.5cm wheelbase, max steering deviation 1.3cm ->
            # max_steering_angle_rad = atan(0.013/0.045) = 0.2812 (16.1deg)
            "wheelbase_m": 0.045,
            "max_steering_angle_rad": 0.2812,
            "lookahead_min_m": 0.08,
            "lookahead_max_m": 0.25,
            "lookahead_speed_gain": 0.02125,
            "stanley_cross_track_gain": 1.0,
            "stanley_softening_speed_mps": 1.0,
            "mpc_horizon_steps": 8,
            "mpc_dt_sec": 0.05,
            "mpc_q_cross_track": 5.0,
            "mpc_q_heading": 3.0,
            "mpc_r_steering": 0.5,
            "mpc_r_steering_rate": 1.0,
            "mpc_optimization_iterations": 15,
            "mpc_initial_search_step_rad": 0.12,
            "mpc_min_search_step_rad": 0.005,
            "max_lateral_accel_mps2": 4.0,
            "v_min_mps": 0.5,
            "v_max_mps": 8.0,
            "speed_p_gain": 1.5,
            "max_steering_rate_rad_s": 3.0,
            "control_rate_hz": 50.0,
        }],
    )
    return LaunchDescription([node])
