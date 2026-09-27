// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__rosidl_typesupport_fastrtps_cpp.hpp.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

#ifndef SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
#define SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_

#include <cstddef>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "safety_manager/msg/rosidl_typesupport_fastrtps_cpp__visibility_control.h"
#include "safety_manager/msg/detail/vehicle_state__struct.hpp"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

#include "fastcdr/Cdr.h"

namespace safety_manager
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
cdr_serialize(
  const safety_manager::msg::VehicleState & ros_message,
  eprosima::fastcdr::Cdr & cdr);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  safety_manager::msg::VehicleState & ros_message);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
get_serialized_size(
  const safety_manager::msg::VehicleState & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
max_serialized_size_VehicleState(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
cdr_serialize_key(
  const safety_manager::msg::VehicleState & ros_message,
  eprosima::fastcdr::Cdr &);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
get_serialized_size_key(
  const safety_manager::msg::VehicleState & ros_message,
  size_t current_alignment);

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
max_serialized_size_key_VehicleState(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace safety_manager

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_safety_manager
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, safety_manager, msg, VehicleState)();

#ifdef __cplusplus
}
#endif

#endif  // SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__ROSIDL_TYPESUPPORT_FASTRTPS_CPP_HPP_
