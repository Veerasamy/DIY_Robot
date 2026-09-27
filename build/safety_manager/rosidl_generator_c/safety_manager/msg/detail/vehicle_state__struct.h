// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/vehicle_state.h"


#ifndef SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__STRUCT_H_
#define SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

/// Constant 'NORMAL'.
enum
{
  safety_manager__msg__VehicleState__NORMAL = 0
};

/// Constant 'AVOID'.
enum
{
  safety_manager__msg__VehicleState__AVOID = 1
};

/// Constant 'BRAKE'.
enum
{
  safety_manager__msg__VehicleState__BRAKE = 2
};

/// Constant 'REVERSE'.
enum
{
  safety_manager__msg__VehicleState__REVERSE = 3
};

/// Constant 'RECOVER'.
enum
{
  safety_manager__msg__VehicleState__RECOVER = 4
};

/// Constant 'EMERGENCY_STOP'.
enum
{
  safety_manager__msg__VehicleState__EMERGENCY_STOP = 5
};

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"
// Member 'reason'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/VehicleState in the package safety_manager.
/**
  * Arbitrated vehicle safety state. safety_manager is the only node allowed
  * to publish this; all actuation nodes must gate their outputs on it.
 */
typedef struct safety_manager__msg__VehicleState
{
  builtin_interfaces__msg__Time stamp;
  uint8_t state;
  rosidl_runtime_c__String reason;
} safety_manager__msg__VehicleState;

// Struct for a sequence of safety_manager__msg__VehicleState.
typedef struct safety_manager__msg__VehicleState__Sequence
{
  safety_manager__msg__VehicleState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} safety_manager__msg__VehicleState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // SAFETY_MANAGER__MSG__DETAIL__VEHICLE_STATE__STRUCT_H_
