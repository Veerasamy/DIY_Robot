// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/vehicle_state.hpp"


#ifndef SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__BUILDER_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "safety_manager/msg/detail/vehicle_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace safety_manager
{

namespace msg
{

namespace builder
{

class Init_VehicleState_reason
{
public:
  explicit Init_VehicleState_reason(::safety_manager::msg::VehicleState & msg)
  : msg_(msg)
  {}
  ::safety_manager::msg::VehicleState reason(::safety_manager::msg::VehicleState::_reason_type arg)
  {
    msg_.reason = std::move(arg);
    return std::move(msg_);
  }

private:
  ::safety_manager::msg::VehicleState msg_;
};

class Init_VehicleState_state
{
public:
  explicit Init_VehicleState_state(::safety_manager::msg::VehicleState & msg)
  : msg_(msg)
  {}
  Init_VehicleState_reason state(::safety_manager::msg::VehicleState::_state_type arg)
  {
    msg_.state = std::move(arg);
    return Init_VehicleState_reason(msg_);
  }

private:
  ::safety_manager::msg::VehicleState msg_;
};

class Init_VehicleState_stamp
{
public:
  Init_VehicleState_stamp()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_VehicleState_state stamp(::safety_manager::msg::VehicleState::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return Init_VehicleState_state(msg_);
  }

private:
  ::safety_manager::msg::VehicleState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::safety_manager::msg::VehicleState>()
{
  return safety_manager::msg::builder::Init_VehicleState_stamp();
}

}  // namespace safety_manager

#endif  // SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__BUILDER_HPP_
