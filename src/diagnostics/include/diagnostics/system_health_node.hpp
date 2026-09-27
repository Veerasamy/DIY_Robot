// Aggregates a topic-staleness watchdog (safety-relevant topics only) and
// Jetson GPU/CPU/RAM utilization into a standard `diagnostic_msgs/
// DiagnosticArray` on `/diagnostics`, viewable with `rqt_runtime_monitor` /
// `rqt_robot_monitor` or logged directly into a rosbag alongside the rest of
// the stack (see `scripts/record_bag.py`).
#pragma once

#include <map>
#include <mutex>
#include <string>

#include "ackermann_msgs/msg/ackermann_drive_stamped.hpp"
#include "diagnostic_msgs/msg/diagnostic_array.hpp"
#include "diagnostics/gpu_cpu_monitor.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "safety_manager/msg/e_stop_status.hpp"
#include "safety_manager/msg/vehicle_state.hpp"

namespace diagnostics
{

class SystemHealthNode : public rclcpp::Node
{
public:
  SystemHealthNode();

private:
  void onPublishTimer();
  diagnostic_msgs::msg::DiagnosticStatus topicWatchdogStatus(
    const std::string & topic, double stale_after_sec) const;
  diagnostic_msgs::msg::DiagnosticStatus gpuCpuDiagnosticStatus() const;

  std::mutex data_mutex_;
  std::map<std::string, rclcpp::Time> last_seen_;

  GpuCpuMonitor gpu_cpu_monitor_;
  double stale_after_sec_{1.0};
  double publish_rate_hz_{2.0};

  rclcpp::Subscription<safety_manager::msg::VehicleState>::SharedPtr vehicle_state_sub_;
  rclcpp::Subscription<safety_manager::msg::EStopStatus>::SharedPtr estop_status_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr cmd_sub_;
  rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr diag_pub_;
  rclcpp::TimerBase::SharedPtr publish_timer_;
  rclcpp::TimerBase::SharedPtr gpu_cpu_timer_;

  GpuCpuSample last_gpu_cpu_sample_;
};

}  // namespace diagnostics
