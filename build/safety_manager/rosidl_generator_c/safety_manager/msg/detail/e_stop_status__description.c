// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice

#include "safety_manager/msg/detail/e_stop_status__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_safety_manager
const rosidl_type_hash_t *
safety_manager__msg__EStopStatus__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0xb5, 0xdc, 0x03, 0xff, 0x0a, 0xd4, 0x43, 0xc1,
      0xa2, 0xbc, 0xca, 0x81, 0xca, 0xcf, 0xf0, 0x49,
      0x9e, 0xcc, 0x17, 0xf7, 0xce, 0x62, 0x3b, 0x8f,
      0xe6, 0x6b, 0x2e, 0x40, 0x2d, 0xcd, 0x3f, 0x70,
    }};
  return &hash;
}

#include <assert.h>
#include <string.h>

// Include directives for referenced types
#include "builtin_interfaces/msg/detail/time__functions.h"

// Hashes for external referenced types
#ifndef NDEBUG
static const rosidl_type_hash_t builtin_interfaces__msg__Time__EXPECTED_HASH = {1, {
    0xb1, 0x06, 0x23, 0x5e, 0x25, 0xa4, 0xc5, 0xed,
    0x35, 0x09, 0x8a, 0xa0, 0xa6, 0x1a, 0x3e, 0xe9,
    0xc9, 0xb1, 0x8d, 0x19, 0x7f, 0x39, 0x8b, 0x0e,
    0x42, 0x06, 0xce, 0xa9, 0xac, 0xf9, 0xc1, 0x97,
  }};
#endif

static char safety_manager__msg__EStopStatus__TYPE_NAME[] = "safety_manager/msg/EStopStatus";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";

// Define type names, field names, and default values
static char safety_manager__msg__EStopStatus__FIELD_NAME__stamp[] = "stamp";
static char safety_manager__msg__EStopStatus__FIELD_NAME__estop_pressed[] = "estop_pressed";
static char safety_manager__msg__EStopStatus__FIELD_NAME__link_ok[] = "link_ok";
static char safety_manager__msg__EStopStatus__FIELD_NAME__radio_ok[] = "radio_ok";
static char safety_manager__msg__EStopStatus__FIELD_NAME__heartbeat_seq[] = "heartbeat_seq";
static char safety_manager__msg__EStopStatus__FIELD_NAME__battery_millivolts[] = "battery_millivolts";
static char safety_manager__msg__EStopStatus__FIELD_NAME__radio_rssi_dbm[] = "radio_rssi_dbm";

static rosidl_runtime_c__type_description__Field safety_manager__msg__EStopStatus__FIELDS[] = {
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__stamp, 5, 5},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__estop_pressed, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__link_ok, 7, 7},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__radio_ok, 8, 8},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_BOOLEAN,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__heartbeat_seq, 13, 13},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT8,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__battery_millivolts, 18, 18},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT16,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__EStopStatus__FIELD_NAME__radio_rssi_dbm, 14, 14},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_INT8,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription safety_manager__msg__EStopStatus__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
safety_manager__msg__EStopStatus__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {safety_manager__msg__EStopStatus__TYPE_NAME, 30, 30},
      {safety_manager__msg__EStopStatus__FIELDS, 7, 7},
    },
    {safety_manager__msg__EStopStatus__REFERENCED_TYPE_DESCRIPTIONS, 1, 1},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "# Decoded status of the physical E-Stop radio link, as reported by the\n"
  "# ESP32 over UART. Published at the UART packet arrival rate (>= 5 Hz).\n"
  "\n"
  "builtin_interfaces/Time stamp\n"
  "\n"
  "# True when the course E-Stop relay is OPEN (button pressed) as read back\n"
  "# on RJ45 pins 5-8 by the team radio and relayed to the robot.\n"
  "bool estop_pressed\n"
  "\n"
  "# True while heartbeat packets are arriving from the ESP32 within the\n"
  "# configured timeout. False is treated as an implicit E-Stop (fail-safe).\n"
  "bool link_ok\n"
  "\n"
  "# True while the team-radio <-> robot-radio RF link itself is healthy\n"
  "# (distinct from the Jetson<->ESP32 UART link captured by link_ok).\n"
  "bool radio_ok\n"
  "\n"
  "uint8 heartbeat_seq\n"
  "uint16 battery_millivolts\n"
  "int8 radio_rssi_dbm";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
safety_manager__msg__EStopStatus__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {safety_manager__msg__EStopStatus__TYPE_NAME, 30, 30},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 709, 709},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
safety_manager__msg__EStopStatus__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[2];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 2, 2};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *safety_manager__msg__EStopStatus__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
