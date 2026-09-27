// ROS2 node: bridges the ESP32 E-Stop radio link (UART) into ROS2 and owns
// the vehicle-wide safety state machine. This is the only node permitted to
// write to the actuator UART link.
#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

#include "ackermann_msgs/msg/ackermann_drive_stamped.hpp"
#include "rclcpp/rclcpp.hpp"
#include "safety_manager/msg/e_stop_status.hpp"
#include "safety_manager/msg/vehicle_state.hpp"
#include "safety_manager/vehicle_state_machine.hpp"
#include "std_msgs/msg/u_int8.hpp"

namespace safety_manager
{

class EStopBridgeNode : public rclcpp::Node
{
public:
  EStopBridgeNode();
  ~EStopBridgeNode() override;

private:
  // --- serial I/O (POSIX termios; Linux/Jetson only) ---
  bool openSerial(const std::string & device, int baud);
  void closeSerial();
  void rxThreadMain();
  void sendWatchdogKick();

  // --- ROS callbacks ---
  void onControlTimer();
  void onStateRequest(const std_msgs::msg::UInt8::SharedPtr msg);
  void onAckermannCmd(const ackermann_msgs::msg::AckermannDriveStamped::SharedPtr msg);

  // --- state ---
  int fd_{-1};
  std::thread rx_thread_;
  std::atomic<bool> running_{false};

  std::mutex status_mutex_;
  bool last_estop_pressed_{true};   // fail-safe default: assume pressed until proven otherwise
  bool last_radio_ok_{false};
  bool link_ok_{false};
  uint8_t last_heartbeat_seq_{0};
  uint16_t last_battery_mv_{0};
  int8_t last_rssi_dbm_{0};
  rclcpp::Time last_packet_time_;

  std::atomic<uint8_t> state_request_{0};  // written by obstacle_detection etc.
  VehicleStateMachine state_machine_;

  rclcpp::Publisher<msg::EStopStatus>::SharedPtr estop_status_pub_;
  rclcpp::Publisher<msg::VehicleState>::SharedPtr vehicle_state_pub_;
  rclcpp::Publisher<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr gated_cmd_pub_;
  rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr state_request_sub_;
  rclcpp::Subscription<ackermann_msgs::msg::AckermannDriveStamped>::SharedPtr cmd_sub_;
  rclcpp::TimerBase::SharedPtr control_timer_;

  // Parameters
  std::string serial_device_;
  int serial_baud_{115200};
  double link_timeout_sec_{0.3};    // 1s hard-stop budget: this must be << 1.0s
  double control_period_sec_{0.02};  // 50 Hz
  double watchdog_kick_period_sec_{0.1};
  rclcpp::Time last_watchdog_kick_time_;
  double last_speed_mps_{0.0};
  double stopped_speed_epsilon_{0.05};
  // Bench-test only: when true, a missing/lost E-Stop UART link is NOT treated
  // as a press. NEVER set true for an actual competition/driving run.
  bool bypass_estop_hardware_{false};
};

}  // namespace safety_manager
