// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice

// IWYU pragma: private, include "safety_manager/msg/e_stop_status.h"


#ifndef SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_H_
#define SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in msg/EStopStatus in the package safety_manager.
/**
  * Decoded status of the physical E-Stop radio link, as reported by the
  * ESP32 over UART. Published at the UART packet arrival rate (>= 5 Hz).
 */
typedef struct safety_manager__msg__EStopStatus
{
  builtin_interfaces__msg__Time stamp;
  /// True when the course E-Stop relay is OPEN (button pressed) as read back
  /// on RJ45 pins 5-8 by the team radio and relayed to the robot.
  bool estop_pressed;
  /// True while heartbeat packets are arriving from the ESP32 within the
  /// configured timeout. False is treated as an implicit E-Stop (fail-safe).
  bool link_ok;
  /// True while the team-radio <-> robot-radio RF link itself is healthy
  /// (distinct from the Jetson<->ESP32 UART link captured by link_ok).
  bool radio_ok;
  uint8_t heartbeat_seq;
  uint16_t battery_millivolts;
  int8_t radio_rssi_dbm;
} safety_manager__msg__EStopStatus;

// Struct for a sequence of safety_manager__msg__EStopStatus.
typedef struct safety_manager__msg__EStopStatus__Sequence
{
  safety_manager__msg__EStopStatus * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} safety_manager__msg__EStopStatus__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // SAFETY_MANAGER__MSG__DETAIL__E_STOP_STATUS__STRUCT_H_
