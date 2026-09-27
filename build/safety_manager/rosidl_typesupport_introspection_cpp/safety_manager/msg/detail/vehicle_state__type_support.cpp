// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "safety_manager/msg/detail/vehicle_state__functions.h"
#include "safety_manager/msg/detail/vehicle_state__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace safety_manager
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void VehicleState_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) safety_manager::msg::VehicleState(_init);
}

void VehicleState_fini_function(void * message_memory)
{
  auto typed_message = static_cast<safety_manager::msg::VehicleState *>(message_memory);
  typed_message->~VehicleState();
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember VehicleState_message_member_array[3] = {
  {
    "stamp",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<builtin_interfaces::msg::Time>(),  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(safety_manager::msg::VehicleState, stamp),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "state",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_UINT8,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(safety_manager::msg::VehicleState, state),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "reason",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is key
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(safety_manager::msg::VehicleState, reason),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers VehicleState_message_members = {
  "safety_manager::msg",  // message namespace
  "VehicleState",  // message name
  3,  // number of fields
  sizeof(safety_manager::msg::VehicleState),
  false,  // has_any_key_member_
  VehicleState_message_member_array,  // message members
  VehicleState_init_function,  // function to initialize message memory (memory has to be allocated)
  VehicleState_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t VehicleState_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &VehicleState_message_members,
  get_message_typesupport_handle_function,
  &safety_manager__msg__VehicleState__get_type_hash,
  &safety_manager__msg__VehicleState__get_type_description,
  &safety_manager__msg__VehicleState__get_type_description_sources,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace safety_manager


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<safety_manager::msg::VehicleState>()
{
  return &::safety_manager::msg::rosidl_typesupport_introspection_cpp::VehicleState_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, safety_manager, msg, VehicleState)() {
  return &::safety_manager::msg::rosidl_typesupport_introspection_cpp::VehicleState_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
