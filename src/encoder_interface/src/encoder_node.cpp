#include "encoder_interface/encoder_node.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>

#include "encoder_interface/gpio_quadrature_reader.hpp"

namespace encoder_interface
{

EncoderNode::EncoderNode()
: Node("encoder_interface")
{
  GpioQuadratureParams gpio_params;
  gpio_params.gpio_chip = declare_parameter<std::string>("gpio_chip", "/dev/gpiochip0");
  gpio_params.pin_a = static_cast<unsigned int>(declare_parameter<int>("gpio_pin_a", 0));
  gpio_params.pin_b = static_cast<unsigned int>(declare_parameter<int>("gpio_pin_b", 0));

  pulses_per_revolution_ = declare_parameter<double>("pulses_per_revolution", 40.0);
  wheel_circumference_m_ = declare_parameter<double>("wheel_circumference_m", 0.2);
  velocity_lowpass_alpha_ = declare_parameter<double>("velocity_lowpass_alpha", 0.3);
  double publish_rate_hz = declare_parameter<double>("publish_rate_hz", 50.0);

#ifdef ENCODER_INTERFACE_HAVE_LIBGPIOD
  reader_ = std::make_unique<GpioQuadratureReader>(gpio_params);
#else
  RCLCPP_WARN(
    get_logger(),
    "encoder_interface built without libgpiod (install libgpiod-dev and rebuild) — "
    "publishing zero ticks/velocity; sensor_fusion still runs on ZED VO+IMU alone.");
  reader_ = std::make_unique<NullEncoderReader>();
#endif

  if (!reader_->start()) {
    RCLCPP_ERROR(get_logger(), "encoder_interface: failed to start GPIO reader, falling back to zero output");
    reader_ = std::make_unique<NullEncoderReader>();
    reader_->start();
  }

  ticks_pub_ = create_publisher<std_msgs::msg::Int32>("/encoder/ticks", rclcpp::QoS(10));
  velocity_pub_ = create_publisher<std_msgs::msg::Float32>("/encoder/velocity_mps", rclcpp::QoS(10));

  last_time_ = now();
  publish_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / publish_rate_hz),
    std::bind(&EncoderNode::onPublishTimer, this));

  RCLCPP_INFO(
    get_logger(), "encoder_interface started, %.1f pulses/rev, %.3fm wheel circumference",
    pulses_per_revolution_, wheel_circumference_m_);
}

void EncoderNode::onPublishTimer()
{
  rclcpp::Time t_now = now();
  double dt = (t_now - last_time_).seconds();
  last_time_ = t_now;
  if (dt <= 0.0) {return;}

  int64_t ticks = reader_->totalTicks();
  int64_t delta_ticks = ticks - last_ticks_;
  last_ticks_ = ticks;

  double raw_velocity_mps =
    (static_cast<double>(delta_ticks) / pulses_per_revolution_) * wheel_circumference_m_ / dt;

  // Simple exponential moving average: the encoder's tick-delta/dt estimate
  // is inherently noisy at low tick counts per publish period (quantization
  // at low speed), and the EKF already discounts this measurement via
  // `encoder_variance` — light smoothing here reduces jitter without adding
  // meaningful lag ahead of that.
  filtered_velocity_mps_ = velocity_lowpass_alpha_ * raw_velocity_mps +
    (1.0 - velocity_lowpass_alpha_) * filtered_velocity_mps_;

  std_msgs::msg::Int32 ticks_msg;
  ticks_msg.data = static_cast<int32_t>(
    std::clamp<int64_t>(
      ticks, std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()));
  ticks_pub_->publish(ticks_msg);

  std_msgs::msg::Float32 velocity_msg;
  velocity_msg.data = static_cast<float>(filtered_velocity_mps_);
  velocity_pub_->publish(velocity_msg);
}

}  // namespace encoder_interface
