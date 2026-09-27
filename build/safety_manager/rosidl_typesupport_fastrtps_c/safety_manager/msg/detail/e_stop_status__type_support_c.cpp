// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice
#include "safety_manager/msg/detail/e_stop_status__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <cstddef>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/serialization_helpers.hpp"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "safety_manager/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "safety_manager/msg/detail/e_stop_status__struct.h"
#include "safety_manager/msg/detail/e_stop_status__functions.h"
#include "fastcdr/Cdr.h"

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

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

#include "builtin_interfaces/msg/detail/time__functions.h"  // stamp

// forward declare type support functions

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
bool cdr_serialize_builtin_interfaces__msg__Time(
  const builtin_interfaces__msg__Time * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
bool cdr_deserialize_builtin_interfaces__msg__Time(
  eprosima::fastcdr::Cdr & cdr,
  builtin_interfaces__msg__Time * ros_message);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
size_t get_serialized_size_builtin_interfaces__msg__Time(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
size_t max_serialized_size_builtin_interfaces__msg__Time(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
bool cdr_serialize_key_builtin_interfaces__msg__Time(
  const builtin_interfaces__msg__Time * ros_message,
  eprosima::fastcdr::Cdr & cdr);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
size_t get_serialized_size_key_builtin_interfaces__msg__Time(
  const void * untyped_ros_message,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
size_t max_serialized_size_key_builtin_interfaces__msg__Time(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

ROSIDL_TYPESUPPORT_FASTRTPS_C_IMPORT_safety_manager
const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, builtin_interfaces, msg, Time)();


using _EStopStatus__ros_msg_type = safety_manager__msg__EStopStatus;


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_serialize_safety_manager__msg__EStopStatus(
  const safety_manager__msg__EStopStatus * ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Field name: stamp
  {
    cdr_serialize_builtin_interfaces__msg__Time(
      &ros_message->stamp, cdr);
  }

  // Field name: estop_pressed
  {
    cdr << (ros_message->estop_pressed ? true : false);
  }

  // Field name: link_ok
  {
    cdr << (ros_message->link_ok ? true : false);
  }

  // Field name: radio_ok
  {
    cdr << (ros_message->radio_ok ? true : false);
  }

  // Field name: heartbeat_seq
  {
    cdr << ros_message->heartbeat_seq;
  }

  // Field name: battery_millivolts
  {
    cdr << ros_message->battery_millivolts;
  }

  // Field name: radio_rssi_dbm
  {
    cdr << ros_message->radio_rssi_dbm;
  }

  return true;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_deserialize_safety_manager__msg__EStopStatus(
  eprosima::fastcdr::Cdr & cdr,
  safety_manager__msg__EStopStatus * ros_message)
{
  // Field name: stamp
  {
    cdr_deserialize_builtin_interfaces__msg__Time(cdr, &ros_message->stamp);
  }

  // Field name: estop_pressed
  {
    uint8_t tmp;
    cdr >> tmp;
    ros_message->estop_pressed = tmp ? true : false;
  }

  // Field name: link_ok
  {
    uint8_t tmp;
    cdr >> tmp;
    ros_message->link_ok = tmp ? true : false;
  }

  // Field name: radio_ok
  {
    uint8_t tmp;
    cdr >> tmp;
    ros_message->radio_ok = tmp ? true : false;
  }

  // Field name: heartbeat_seq
  {
    cdr >> ros_message->heartbeat_seq;
  }

  // Field name: battery_millivolts
  {
    cdr >> ros_message->battery_millivolts;
  }

  // Field name: radio_rssi_dbm
  {
    cdr >> ros_message->radio_rssi_dbm;
  }

  return true;
}  // NOLINT(readability/fn_size)


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t get_serialized_size_safety_manager__msg__EStopStatus(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _EStopStatus__ros_msg_type * ros_message = static_cast<const _EStopStatus__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Field name: stamp
  current_alignment += get_serialized_size_builtin_interfaces__msg__Time(
    &(ros_message->stamp), current_alignment);

  // Field name: estop_pressed
  {
    size_t item_size = sizeof(ros_message->estop_pressed);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: link_ok
  {
    size_t item_size = sizeof(ros_message->link_ok);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: radio_ok
  {
    size_t item_size = sizeof(ros_message->radio_ok);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: heartbeat_seq
  {
    size_t item_size = sizeof(ros_message->heartbeat_seq);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: battery_millivolts
  {
    size_t item_size = sizeof(ros_message->battery_millivolts);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: radio_rssi_dbm
  {
    size_t item_size = sizeof(ros_message->radio_rssi_dbm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}


ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t max_serialized_size_safety_manager__msg__EStopStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // Field name: stamp
  {
    size_t array_size = 1;
    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_builtin_interfaces__msg__Time(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Field name: estop_pressed
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: link_ok
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: radio_ok
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: heartbeat_seq
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: battery_millivolts
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint16_t);
    current_alignment += array_size * sizeof(uint16_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint16_t));
  }

  // Field name: radio_rssi_dbm
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }


  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = safety_manager__msg__EStopStatus;
    is_plain =
      (
      offsetof(DataType, radio_rssi_dbm) +
      last_member_size
      ) == ret_val;
  }
  return ret_val;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
bool cdr_serialize_key_safety_manager__msg__EStopStatus(
  const safety_manager__msg__EStopStatus * ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Field name: stamp
  {
    cdr_serialize_key_builtin_interfaces__msg__Time(
      &ros_message->stamp, cdr);
  }

  // Field name: estop_pressed
  {
    cdr << (ros_message->estop_pressed ? true : false);
  }

  // Field name: link_ok
  {
    cdr << (ros_message->link_ok ? true : false);
  }

  // Field name: radio_ok
  {
    cdr << (ros_message->radio_ok ? true : false);
  }

  // Field name: heartbeat_seq
  {
    cdr << ros_message->heartbeat_seq;
  }

  // Field name: battery_millivolts
  {
    cdr << ros_message->battery_millivolts;
  }

  // Field name: radio_rssi_dbm
  {
    cdr << ros_message->radio_rssi_dbm;
  }

  return true;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t get_serialized_size_key_safety_manager__msg__EStopStatus(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _EStopStatus__ros_msg_type * ros_message = static_cast<const _EStopStatus__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;

  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Field name: stamp
  current_alignment += get_serialized_size_key_builtin_interfaces__msg__Time(
    &(ros_message->stamp), current_alignment);

  // Field name: estop_pressed
  {
    size_t item_size = sizeof(ros_message->estop_pressed);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: link_ok
  {
    size_t item_size = sizeof(ros_message->link_ok);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: radio_ok
  {
    size_t item_size = sizeof(ros_message->radio_ok);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: heartbeat_seq
  {
    size_t item_size = sizeof(ros_message->heartbeat_seq);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: battery_millivolts
  {
    size_t item_size = sizeof(ros_message->battery_millivolts);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  // Field name: radio_rssi_dbm
  {
    size_t item_size = sizeof(ros_message->radio_rssi_dbm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_safety_manager
size_t max_serialized_size_key_safety_manager__msg__EStopStatus(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;
  // Field name: stamp
  {
    size_t array_size = 1;
    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_key_builtin_interfaces__msg__Time(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Field name: estop_pressed
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: link_ok
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: radio_ok
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: heartbeat_seq
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Field name: battery_millivolts
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint16_t);
    current_alignment += array_size * sizeof(uint16_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint16_t));
  }

  // Field name: radio_rssi_dbm
  {
    size_t array_size = 1;
    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = safety_manager__msg__EStopStatus;
    is_plain =
      (
      offsetof(DataType, radio_rssi_dbm) +
      last_member_size
      ) == ret_val;
  }
  return ret_val;
}


static bool _EStopStatus__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const safety_manager__msg__EStopStatus * ros_message = static_cast<const safety_manager__msg__EStopStatus *>(untyped_ros_message);
  (void)ros_message;
  return cdr_serialize_safety_manager__msg__EStopStatus(ros_message, cdr);
}

static bool _EStopStatus__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  safety_manager__msg__EStopStatus * ros_message = static_cast<safety_manager__msg__EStopStatus *>(untyped_ros_message);
  (void)ros_message;
  return cdr_deserialize_safety_manager__msg__EStopStatus(cdr, ros_message);
}

static uint32_t _EStopStatus__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_safety_manager__msg__EStopStatus(
      untyped_ros_message, 0));
}

static size_t _EStopStatus__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_safety_manager__msg__EStopStatus(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_EStopStatus = {
  "safety_manager::msg",
  "EStopStatus",
  _EStopStatus__cdr_serialize,
  _EStopStatus__cdr_deserialize,
  _EStopStatus__get_serialized_size,
  _EStopStatus__max_serialized_size,
  nullptr
};

static rosidl_message_type_support_t _EStopStatus__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_EStopStatus,
  get_message_typesupport_handle_function,
  &safety_manager__msg__EStopStatus__get_type_hash,
  &safety_manager__msg__EStopStatus__get_type_description,
  &safety_manager__msg__EStopStatus__get_type_description_sources,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, safety_manager, msg, EStopStatus)() {
  return &_EStopStatus__type_support;
}

#if defined(__cplusplus)
}
#endif
