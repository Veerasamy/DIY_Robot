#include "diagnostics/system_health_node.hpp"

#include "diagnostic_msgs/msg/diagnostic_status.hpp"
#include "diagnostic_msgs/msg/key_value.hpp"

namespace diagnostics
{

SystemHealthNode::SystemHealthNode()
: Node("system_health")
{
  stale_after_sec_ = declare_parameter<double>("stale_after_sec", 1.0);
  publish_rate_hz_ = declare_parameter<double>("publish_rate_hz", 2.0);
  double gpu_cpu_rate_hz = declare_parameter<double>("gpu_cpu_sample_rate_hz", 0.5);

  auto touch = [this](const std::string & topic) {
      std::lock_guard<std::mutex> lock(data_mutex_);
      last_seen_[topic] = now();
    };

  vehicle_state_sub_ = create_subscription<safety_manager::msg::VehicleState>(
    "/safety/vehicle_state", rclcpp::QoS(10),
    [touch](const safety_manager::msg::VehicleState::ConstSharedPtr &) {
      touch("/safety/vehicle_state");
    });
  estop_status_sub_ = create_subscription<safety_manager::msg::EStopStatus>(
    "/safety/estop_status", rclcpp::QoS(10),
    [touch](const safety_manager::msg::EStopStatus::ConstSharedPtr &) {
      touch("/safety/estop_status");
    });
  odom_sub_ = create_subscription<nav_msgs::msg::Odometry>(
    "/ekf/odom", rclcpp::QoS(10),
    [touch](const nav_msgs::msg::Odometry::ConstSharedPtr &) {touch("/ekf/odom");});
  cmd_sub_ = create_subscription<ackermann_msgs::msg::AckermannDriveStamped>(
    "/cmd_ackermann", rclcpp::QoS(10),
    [touch](const ackermann_msgs::msg::AckermannDriveStamped::ConstSharedPtr &) {
      touch("/cmd_ackermann");
    });

  diag_pub_ = create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/diagnostics", rclcpp::QoS(10));

  publish_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / publish_rate_hz_),
    std::bind(&SystemHealthNode::onPublishTimer, this));

  // GPU/CPU sampling shells out to `tegrastats` (bounded to ~2s per call, see
  // GpuCpuMonitor) -- run it on its own slower timer so it never delays the
  // topic-watchdog publish cadence above.
  gpu_cpu_timer_ = create_wall_timer(
    std::chrono::duration<double>(1.0 / gpu_cpu_rate_hz),
    [this]() {
      std::lock_guard<std::mutex> lock(data_mutex_);
      last_gpu_cpu_sample_ = gpu_cpu_monitor_.sample();
    });

  RCLCPP_INFO(get_logger(), "diagnostics (system_health) started");
}

diagnostic_msgs::msg::DiagnosticStatus SystemHealthNode::topicWatchdogStatus(
  const std::string & topic, double stale_after_sec) const
{
  diagnostic_msgs::msg::DiagnosticStatus status;
  status.name = "topic: " + topic;
  status.hardware_id = "autonomous_rc";

  auto it = last_seen_.find(topic);
  if (it == last_seen_.end()) {
    status.level = diagnostic_msgs::msg::DiagnosticStatus::WARN;
    status.message = "no messages received yet";
    return status;
  }

  double age_sec = (now() - it->second).seconds();
  if (age_sec > stale_after_sec) {
    status.level = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
    status.message = "stale (" + std::to_string(age_sec) + "s since last message)";
  } else {
    status.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
    status.message = "receiving";
  }

  diagnostic_msgs::msg::KeyValue kv;
  kv.key = "age_sec";
  kv.value = std::to_string(age_sec);
  status.values.push_back(kv);
  return status;
}

diagnostic_msgs::msg::DiagnosticStatus SystemHealthNode::gpuCpuDiagnosticStatus() const
{
  diagnostic_msgs::msg::DiagnosticStatus status;
  status.name = "jetson_gpu_cpu";
  status.hardware_id = "orin_nano";

  if (!last_gpu_cpu_sample_.available) {
    status.level = diagnostic_msgs::msg::DiagnosticStatus::WARN;
    status.message = "tegrastats unavailable (not running on a Jetson, or binary not found)";
    return status;
  }

  status.level = diagnostic_msgs::msg::DiagnosticStatus::OK;
  status.message = "ok";

  auto addKv = [&status](const std::string & key, double value) {
      diagnostic_msgs::msg::KeyValue kv;
      kv.key = key;
      kv.value = std::to_string(value);
      status.values.push_back(kv);
    };
  addKv("cpu_percent", last_gpu_cpu_sample_.cpu_percent);
  addKv("gpu_percent", last_gpu_cpu_sample_.gpu_percent);
  addKv("ram_used_mb", last_gpu_cpu_sample_.ram_used_mb);
  addKv("ram_total_mb", last_gpu_cpu_sample_.ram_total_mb);
  return status;
}

void SystemHealthNode::onPublishTimer()
{
  diagnostic_msgs::msg::DiagnosticArray array;
  array.header.stamp = now();

  {
    std::lock_guard<std::mutex> lock(data_mutex_);
    array.status.push_back(topicWatchdogStatus("/safety/vehicle_state", stale_after_sec_));
    array.status.push_back(topicWatchdogStatus("/safety/estop_status", stale_after_sec_));
    array.status.push_back(topicWatchdogStatus("/ekf/odom", stale_after_sec_));
    array.status.push_back(topicWatchdogStatus("/cmd_ackermann", stale_after_sec_));
    array.status.push_back(gpuCpuDiagnosticStatus());
  }

  diag_pub_->publish(array);
}

}  // namespace diagnostics
