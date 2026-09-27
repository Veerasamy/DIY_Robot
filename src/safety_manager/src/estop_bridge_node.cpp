#include "safety_manager/estop_bridge_node.hpp"

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <vector>

#include "uart_protocol.hpp"  // from safety_manager/common — shared with ESP32 firmware

namespace safety_manager
{

using namespace std::chrono_literals;

EStopBridgeNode::EStopBridgeNode()
: Node("safety_manager")
{
  serial_device_ = declare_parameter<std::string>("serial_device", "/dev/ttyUSB_estop");
  serial_baud_ = declare_parameter<int>("serial_baud", 115200);
  link_timeout_sec_ = declare_parameter<double>("link_timeout_sec", 0.3);
  control_period_sec_ = declare_parameter<double>("control_period_sec", 0.02);
  watchdog_kick_period_sec_ = declare_parameter<double>("watchdog_kick_period_sec", 0.1);
  stopped_speed_epsilon_ = declare_parameter<double>("stopped_speed_epsilon", 0.05);
  bypass_estop_hardware_ = declare_parameter<bool>("bypass_estop_hardware", false);

  estop_status_pub_ = create_publisher<msg::EStopStatus>("/safety/estop_status", rclcpp::QoS(10));
  vehicle_state_pub_ = create_publisher<msg::VehicleState>("/safety/vehicle_state", rclcpp::QoS(10));
  gated_cmd_pub_ = create_publisher<ackermann_msgs::msg::AckermannDriveStamped>(
    "/safety/gated_cmd_ackermann", rclcpp::QoS(10));

  state_request_sub_ = create_subscription<std_msgs::msg::UInt8>(
    "/safety/state_request", rclcpp::QoS(10),
    std::bind(&EStopBridgeNode::onStateRequest, this, std::placeholders::_1));

  cmd_sub_ = create_subscription<ackermann_msgs::msg::AckermannDriveStamped>(
    "/cmd_ackermann", rclcpp::QoS(10),
    std::bind(&EStopBridgeNode::onAckermannCmd, this, std::placeholders::_1));

  last_packet_time_ = now();
  last_watchdog_kick_time_ = now();

  if (bypass_estop_hardware_) {
    RCLCPP_WARN(
      get_logger(),
      "*** bypass_estop_hardware=true: E-Stop hardware link is DISABLED. "
      "This vehicle will NOT stop on hardware E-Stop press or link loss. "
      "Bench-testing only — never use for an actual driving run. ***");
  }

  if (!openSerial(serial_device_, serial_baud_)) {
    RCLCPP_ERROR(
      get_logger(),
      "Failed to open E-Stop serial link '%s' — vehicle will remain latched in "
      "EMERGENCY_STOP until the link is established (fail-safe default).%s",
      serial_device_.c_str(),
      bypass_estop_hardware_ ? " (bypassed — ignoring)" : "");
  } else {
    running_ = true;
    rx_thread_ = std::thread(&EStopBridgeNode::rxThreadMain, this);
  }

  control_timer_ = create_wall_timer(
    std::chrono::duration<double>(control_period_sec_),
    std::bind(&EStopBridgeNode::onControlTimer, this));

  RCLCPP_INFO(get_logger(), "safety_manager started (link_timeout=%.2fs, budget<1.0s)",
    link_timeout_sec_);
}

EStopBridgeNode::~EStopBridgeNode()
{
  running_ = false;
  if (rx_thread_.joinable()) {rx_thread_.join();}
  closeSerial();
}

bool EStopBridgeNode::openSerial(const std::string & device, int baud)
{
  fd_ = ::open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if (fd_ < 0) {
    RCLCPP_WARN(get_logger(), "open(%s) failed: %s", device.c_str(), std::strerror(errno));
    return false;
  }

  termios tty{};
  if (tcgetattr(fd_, &tty) != 0) {
    RCLCPP_WARN(get_logger(), "tcgetattr failed: %s", std::strerror(errno));
    ::close(fd_);
    fd_ = -1;
    return false;
  }

  speed_t speed = (baud == 921600) ? B921600 : (baud == 57600 ? B57600 : B115200);
  cfsetispeed(&tty, speed);
  cfsetospeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
  tty.c_iflag &= ~IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;  // 100ms read timeout granularity
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~(PARENB | PARODD);
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    RCLCPP_WARN(get_logger(), "tcsetattr failed: %s", std::strerror(errno));
    ::close(fd_);
    fd_ = -1;
    return false;
  }
  return true;
}

void EStopBridgeNode::closeSerial()
{
  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

void EStopBridgeNode::rxThreadMain()
{
  std::vector<uint8_t> ring;
  ring.reserve(64);
  uint8_t byte;

  while (running_) {
    ssize_t n = ::read(fd_, &byte, 1);
    if (n <= 0) {
      std::this_thread::sleep_for(2ms);
      continue;
    }

    // Resync on start byte to stay robust against partial/garbled frames
    // from a marginal 300ft radio hop.
    if (ring.empty() && byte != uart::kStartByte) {
      continue;
    }
    ring.push_back(byte);

    if (ring.size() == uart::kStatusPacketLen) {
      uart::StatusPacket pkt{};
      if (uart::decodeStatus(ring.data(), ring.size(), pkt)) {
        std::lock_guard<std::mutex> lock(status_mutex_);
        last_estop_pressed_ = pkt.estop_pressed;
        last_radio_ok_ = pkt.radio_ok;
        last_heartbeat_seq_ = pkt.heartbeat_seq;
        last_battery_mv_ = pkt.battery_millivolts;
        last_rssi_dbm_ = pkt.radio_rssi_dbm;
        last_packet_time_ = now();
        link_ok_ = true;
      } else {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 1000, "CRC/frame error on E-Stop UART link");
      }
      ring.clear();
    }
  }
}

void EStopBridgeNode::sendWatchdogKick()
{
  if (fd_ < 0) {return;}
  static uint8_t seq = 0;
  auto buf = uart::encodeCommand(uart::kCmdWatchdogKick, seq++);
  ::write(fd_, buf.data(), buf.size());
}

void EStopBridgeNode::onStateRequest(const std_msgs::msg::UInt8::SharedPtr msg)
{
  state_request_.store(msg->data);
}

void EStopBridgeNode::onAckermannCmd(const ackermann_msgs::msg::AckermannDriveStamped::SharedPtr msg)
{
  last_speed_mps_ = msg->drive.speed;

  bool blocked = state_machine_.state() == VehicleState::kEmergencyStop ||
    state_machine_.state() == VehicleState::kBrake;

  ackermann_msgs::msg::AckermannDriveStamped out = *msg;
  if (blocked) {
    out.drive.speed = 0.0;
    out.drive.acceleration = 0.0;
  }
  gated_cmd_pub_->publish(out);
}

void EStopBridgeNode::onControlTimer()
{
  const rclcpp::Time t_now = now();

  bool estop_pressed;
  bool radio_ok;
  uint8_t heartbeat_seq;
  uint16_t battery_mv;
  int8_t rssi;
  bool packet_fresh;
  {
    std::lock_guard<std::mutex> lock(status_mutex_);
    double age = (t_now - last_packet_time_).seconds();
    packet_fresh = link_ok_ && age <= link_timeout_sec_;
    estop_pressed = last_estop_pressed_;
    radio_ok = last_radio_ok_;
    heartbeat_seq = last_heartbeat_seq_;
    battery_mv = last_battery_mv_;
    rssi = last_rssi_dbm_;
  }

  // Fail-safe: any loss of the UART heartbeat within the 1-second competition
  // budget is treated as an E-Stop press, matching "robot must stop within
  // 1 second" even if the radio link itself silently dies.
  // bypass_estop_hardware_ disables this fail-safe for bench testing without
  // E-Stop hardware attached (no link ever comes up in that case, so the
  // default fail-safe last_estop_pressed_=true would otherwise latch forever)
  // — a real link, if one does connect, still reports its true estop_pressed.
  bool effective_estop = packet_fresh ? estop_pressed : !bypass_estop_hardware_;

  StateInputs in;
  in.estop_pressed = effective_estop;
  in.vehicle_stopped = std::abs(last_speed_mps_) < stopped_speed_epsilon_;
  uint8_t req = state_request_.load();
  in.obstacle_close = req == 1;
  in.collision_imminent = req == 2;
  in.path_clear = req == 3;
  in.recovered = req == 4;
  in.operator_clear_requested = req == 9;

  VehicleState vs = state_machine_.update(in);

  msg::EStopStatus status_msg;
  status_msg.stamp = t_now;
  status_msg.estop_pressed = effective_estop;
  status_msg.link_ok = packet_fresh;
  status_msg.radio_ok = radio_ok;
  status_msg.heartbeat_seq = heartbeat_seq;
  status_msg.battery_millivolts = battery_mv;
  status_msg.radio_rssi_dbm = rssi;
  estop_status_pub_->publish(status_msg);

  msg::VehicleState vs_msg;
  vs_msg.stamp = t_now;
  vs_msg.state = static_cast<uint8_t>(vs);
  vs_msg.reason = state_machine_.reason();
  vehicle_state_pub_->publish(vs_msg);

  if ((t_now - last_watchdog_kick_time_).seconds() >= watchdog_kick_period_sec_) {
    sendWatchdogKick();
    last_watchdog_kick_time_ = t_now;
  }
}

}  // namespace safety_manager
