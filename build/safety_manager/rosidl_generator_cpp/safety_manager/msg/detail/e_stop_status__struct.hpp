// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/e_stop_status.hpp"


#ifndef SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__safety_manager__msg__EStopStatus __attribute__((deprecated))
#else
# define DEPRECATED__safety_manager__msg__EStopStatus __declspec(deprecated)
#endif

namespace safety_manager
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct EStopStatus_
{
  using Type = EStopStatus_<ContainerAllocator>;

  explicit EStopStatus_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->estop_pressed = false;
      this->link_ok = false;
      this->radio_ok = false;
      this->heartbeat_seq = 0;
      this->battery_millivolts = 0;
      this->radio_rssi_dbm = 0;
    }
  }

  explicit EStopStatus_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : stamp(_alloc, _init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->estop_pressed = false;
      this->link_ok = false;
      this->radio_ok = false;
      this->heartbeat_seq = 0;
      this->battery_millivolts = 0;
      this->radio_rssi_dbm = 0;
    }
  }

  // field types and members
  using _stamp_type =
    builtin_interfaces::msg::Time_<ContainerAllocator>;
  _stamp_type stamp;
  using _estop_pressed_type =
    bool;
  _estop_pressed_type estop_pressed;
  using _link_ok_type =
    bool;
  _link_ok_type link_ok;
  using _radio_ok_type =
    bool;
  _radio_ok_type radio_ok;
  using _heartbeat_seq_type =
    uint8_t;
  _heartbeat_seq_type heartbeat_seq;
  using _battery_millivolts_type =
    uint16_t;
  _battery_millivolts_type battery_millivolts;
  using _radio_rssi_dbm_type =
    int8_t;
  _radio_rssi_dbm_type radio_rssi_dbm;

  // setters for named parameter idiom
  Type & set__stamp(
    const builtin_interfaces::msg::Time_<ContainerAllocator> & _arg)
  {
    this->stamp = _arg;
    return *this;
  }
  Type & set__estop_pressed(
    const bool & _arg)
  {
    this->estop_pressed = _arg;
    return *this;
  }
  Type & set__link_ok(
    const bool & _arg)
  {
    this->link_ok = _arg;
    return *this;
  }
  Type & set__radio_ok(
    const bool & _arg)
  {
    this->radio_ok = _arg;
    return *this;
  }
  Type & set__heartbeat_seq(
    const uint8_t & _arg)
  {
    this->heartbeat_seq = _arg;
    return *this;
  }
  Type & set__battery_millivolts(
    const uint16_t & _arg)
  {
    this->battery_millivolts = _arg;
    return *this;
  }
  Type & set__radio_rssi_dbm(
    const int8_t & _arg)
  {
    this->radio_rssi_dbm = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    safety_manager::msg::EStopStatus_<ContainerAllocator> *;
  using ConstRawPtr =
    const safety_manager::msg::EStopStatus_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      safety_manager::msg::EStopStatus_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      safety_manager::msg::EStopStatus_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__safety_manager__msg__EStopStatus
    std::shared_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__safety_manager__msg__EStopStatus
    std::shared_ptr<safety_manager::msg::EStopStatus_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const EStopStatus_ & other) const
  {
    if (this->stamp != other.stamp) {
      return false;
    }
    if (this->estop_pressed != other.estop_pressed) {
      return false;
    }
    if (this->link_ok != other.link_ok) {
      return false;
    }
    if (this->radio_ok != other.radio_ok) {
      return false;
    }
    if (this->heartbeat_seq != other.heartbeat_seq) {
      return false;
    }
    if (this->battery_millivolts != other.battery_millivolts) {
      return false;
    }
    if (this->radio_rssi_dbm != other.radio_rssi_dbm) {
      return false;
    }
    return true;
  }
  bool operator!=(const EStopStatus_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct EStopStatus_

// alias to use template instance with default allocator
using EStopStatus =
  safety_manager::msg::EStopStatus_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace safety_manager

#endif  // SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_HPP_
