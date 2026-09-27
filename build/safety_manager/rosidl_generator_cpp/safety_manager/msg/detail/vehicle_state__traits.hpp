// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/vehicle_state.hpp"


#ifndef SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__TRAITS_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "safety_manager/msg/detail/vehicle_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__traits.hpp"

namespace safety_manager
{

namespace msg
{

inline void to_flow_style_yaml(
  const VehicleState & msg,
  std::ostream & out)
{
  out << "{";
  // member: stamp
  {
    out << "stamp: ";
    to_flow_style_yaml(msg.stamp, out);
    out << ", ";
  }

  // member: state
  {
    out << "state: ";
    rosidl_generator_traits::value_to_yaml(msg.state, out);
    out << ", ";
  }

  // member: reason
  {
    out << "reason: ";
    rosidl_generator_traits::value_to_yaml(msg.reason, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const VehicleState & msg,
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

  // member: state
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "state: ";
    rosidl_generator_traits::value_to_yaml(msg.state, out);
    out << "\n";
  }

  // member: reason
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "reason: ";
    rosidl_generator_traits::value_to_yaml(msg.reason, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const VehicleState & msg, bool use_flow_style = false)
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
  const safety_manager::msg::VehicleState & msg,
  std::ostream & out, size_t indentation = 0)
{
  safety_manager::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use safety_manager::msg::to_yaml() instead")]]
inline std::string to_yaml(const safety_manager::msg::VehicleState & msg)
{
  return safety_manager::msg::to_yaml(msg);
}

template<>
inline const char * data_type<safety_manager::msg::VehicleState>()
{
  return "safety_manager::msg::VehicleState";
}

template<>
inline const char * name<safety_manager::msg::VehicleState>()
{
  return "safety_manager/msg/VehicleState";
}

template<>
struct has_fixed_size<safety_manager::msg::VehicleState>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<safety_manager::msg::VehicleState>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<safety_manager::msg::VehicleState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__TRAITS_HPP_
