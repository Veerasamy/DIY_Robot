// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/e_stop_status.hpp"


#ifndef SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__BUILDER_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "safety_manager/msg/detail/e_stop_status__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace safety_manager
{

namespace msg
{

namespace builder
{

class Init_EStopStatus_radio_rssi_dbm
{
public:
  explicit Init_EStopStatus_radio_rssi_dbm(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  ::safety_manager::msg::EStopStatus radio_rssi_dbm(::safety_manager::msg::EStopStatus::_radio_rssi_dbm_type arg)
  {
    msg_.radio_rssi_dbm = std::move(arg);
    return std::move(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_battery_millivolts
{
public:
  explicit Init_EStopStatus_battery_millivolts(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  Init_EStopStatus_radio_rssi_dbm battery_millivolts(::safety_manager::msg::EStopStatus::_battery_millivolts_type arg)
  {
    msg_.battery_millivolts = std::move(arg);
    return Init_EStopStatus_radio_rssi_dbm(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_heartbeat_seq
{
public:
  explicit Init_EStopStatus_heartbeat_seq(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  Init_EStopStatus_battery_millivolts heartbeat_seq(::safety_manager::msg::EStopStatus::_heartbeat_seq_type arg)
  {
    msg_.heartbeat_seq = std::move(arg);
    return Init_EStopStatus_battery_millivolts(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_radio_ok
{
public:
  explicit Init_EStopStatus_radio_ok(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  Init_EStopStatus_heartbeat_seq radio_ok(::safety_manager::msg::EStopStatus::_radio_ok_type arg)
  {
    msg_.radio_ok = std::move(arg);
    return Init_EStopStatus_heartbeat_seq(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_link_ok
{
public:
  explicit Init_EStopStatus_link_ok(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  Init_EStopStatus_radio_ok link_ok(::safety_manager::msg::EStopStatus::_link_ok_type arg)
  {
    msg_.link_ok = std::move(arg);
    return Init_EStopStatus_radio_ok(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_estop_pressed
{
public:
  explicit Init_EStopStatus_estop_pressed(::safety_manager::msg::EStopStatus & msg)
  : msg_(msg)
  {}
  Init_EStopStatus_link_ok estop_pressed(::safety_manager::msg::EStopStatus::_estop_pressed_type arg)
  {
    msg_.estop_pressed = std::move(arg);
    return Init_EStopStatus_link_ok(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

class Init_EStopStatus_stamp
{
public:
  Init_EStopStatus_stamp()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_EStopStatus_estop_pressed stamp(::safety_manager::msg::EStopStatus::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return Init_EStopStatus_estop_pressed(msg_);
  }

private:
  ::safety_manager::msg::EStopStatus msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::safety_manager::msg::EStopStatus>()
{
  return safety_manager::msg::builder::Init_EStopStatus_stamp();
}

}  // namespace safety_manager

#endif  // SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__BUILDER_HPP_
