// ROS2 node: publishes the wheel encoder's cumulative tick count and a
// filtered forward-speed estimate consumed by `sensor_fusion`'s EKF
// (`/encoder/velocity_mps`, per ARCHITECTURE.md's topic contract).
#pragma once

#include <memory>

#include "encoder_interface/encoder_reader_interface.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/int32.hpp"

namespace encoder_interface
{

class EncoderNode : public rclcpp::Node
{
public:
  EncoderNode();

private:
  void onPublishTimer();

  std::unique_ptr<IEncoderReader> reader_;

  double pulses_per_revolution_{40.0};  // placeholder — MEASURE for the SS49E wheel target count
  double wheel_circumference_m_{0.2};   // placeholder — MEASURE the SCRT10's wheel diameter
  double velocity_lowpass_alpha_{0.3};  // EMA smoothing factor, 0=frozen .. 1=unfiltered

  int64_t last_ticks_{0};
  rclcpp::Time last_time_;
  double filtered_velocity_mps_{0.0};

  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr ticks_pub_;
  rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr velocity_pub_;
  rclcpp::TimerBase::SharedPtr publish_timer_;
};

}  // namespace encoder_interface
