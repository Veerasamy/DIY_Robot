// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/e_stop_status.hpp"


#ifndef SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__TRAITS_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "safety_manager/msg/detail/e_stop_status__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__traits.hpp"

namespace safety_manager
{

namespace msg
{

inline void to_flow_style_yaml(
  const EStopStatus & msg,
  std::ostream & out)
{
  out << "{";
  // member: stamp
  {
    out << "stamp: ";
    to_flow_style_yaml(msg.stamp, out);
    out << ", ";
  }

  // member: estop_pressed
  {
    out << "estop_pressed: ";
    rosidl_generator_traits::value_to_yaml(msg.estop_pressed, out);
    out << ", ";
  }

  // member: link_ok
  {
    out << "link_ok: ";
    rosidl_generator_traits::value_to_yaml(msg.link_ok, out);
    out << ", ";
  }

  // member: radio_ok
  {
    out << "radio_ok: ";
    rosidl_generator_traits::value_to_yaml(msg.radio_ok, out);
    out << ", ";
  }

  // member: heartbeat_seq
  {
    out << "heartbeat_seq: ";
    rosidl_generator_traits::value_to_yaml(msg.heartbeat_seq, out);
    out << ", ";
  }

  // member: battery_millivolts
  {
    out << "battery_millivolts: ";
    rosidl_generator_traits::value_to_yaml(msg.battery_millivolts, out);
    out << ", ";
  }

  // member: radio_rssi_dbm
  {
    out << "radio_rssi_dbm: ";
    rosidl_generator_traits::value_to_yaml(msg.radio_rssi_dbm, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const EStopStatus & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: stamp
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "stamp:\n";
    to_block_style_yaml(msg.stamp, out, indentation + 2);
  }

  // member: estop_pressed
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "estop_pressed: ";
    rosidl_generator_traits::value_to_yaml(msg.estop_pressed, out);
    out << "\n";
  }

  // member: link_ok
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "link_ok: ";
    rosidl_generator_traits::value_to_yaml(msg.link_ok, out);
    out << "\n";
  }

  // member: radio_ok
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "radio_ok: ";
    rosidl_generator_traits::value_to_yaml(msg.radio_ok, out);
    out << "\n";
  }

  // member: heartbeat_seq
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "heartbeat_seq: ";
    rosidl_generator_traits::value_to_yaml(msg.heartbeat_seq, out);
    out << "\n";
  }

  // member: battery_millivolts
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "battery_millivolts: ";
    rosidl_generator_traits::value_to_yaml(msg.battery_millivolts, out);
    out << "\n";
  }

  // member: radio_rssi_dbm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "radio_rssi_dbm: ";
    rosidl_generator_traits::value_to_yaml(msg.radio_rssi_dbm, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const EStopStatus & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace safety_manager

namespace rosidl_generator_traits
{

[[deprecated("use safety_manager::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const safety_manager::msg::EStopStatus & msg,
  std::ostream & out, size_t indentation = 0)
{
  safety_manager::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use safety_manager::msg::to_yaml() instead")]]
inline std::string to_yaml(const safety_manager::msg::EStopStatus & msg)
{
  return safety_manager::msg::to_yaml(msg);
}

template<>
inline const char * data_type<safety_manager::msg::EStopStatus>()
{
  return "safety_manager::msg::EStopStatus";
}

template<>
inline const char * name<safety_manager::msg::EStopStatus>()
{
  return "safety_manager/msg/EStopStatus";
}

template<>
struct has_fixed_size<safety_manager::msg::EStopStatus>
  : std::integral_constant<bool, has_fixed_size<builtin_interfaces::msg::Time>::value> {};

template<>
struct has_bounded_size<safety_manager::msg::EStopStatus>
  : std::integral_constant<bool, has_bounded_size<builtin_interfaces::msg::Time>::value> {};

template<>
struct is_message<safety_manager::msg::EStopStatus>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__TRAITS_HPP_
