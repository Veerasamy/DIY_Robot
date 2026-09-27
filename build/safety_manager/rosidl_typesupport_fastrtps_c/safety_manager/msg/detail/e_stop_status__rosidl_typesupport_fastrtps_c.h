// generated from rosidl_typesupport_fastrtps_c/resource/idl__rosidl_typesupport_fastrtps_c.h.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice
#ifndef SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
#define SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_


#include <stddef.h>
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_interface/macros.h"
#include "safety_manager/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "safety_manager/msg/detail/e_stop_status__struct.h"
#include "fastcdr/Cdr.h"

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_serialize_safety_manager__msg__EStopStatus(
  const safety_manager__msg__EStopStatus * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_deserialize_safety_manager__msg__EStopStatus(
  eprosima::fastcdr::Cdr &,
  safety_manager__msg__EStopStatus * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t get_serialized_size_safety_manager__msg__EStopStatus(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t max_serialized_size_safety_manager__msg__EStopStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_serialize_key_safety_manager__msg__EStopStatus(
  const safety_manager__msg__EStopStatus * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t get_serialized_size_key_safety_manager__msg__EStopStatus(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t max_serialized_size_key_safety_manager__msg__EStopStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, safety_manager, msg, EStopStatus)();

#ifdef __cplusplus
}
#endif

#endif  // SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__ROSIDL_TYPESUPPORT_FASTRTPS_C_H_
