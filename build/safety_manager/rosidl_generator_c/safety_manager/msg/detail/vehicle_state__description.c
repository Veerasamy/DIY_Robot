// generated from rosidl_generator_c/resource/idl__description.c.em
// with input from safety_manager:msg/VehicleState.idl
// generated code does not contain a copyright notice

#include "safety_manager/msg/detail/vehicle_state__functions.h"

ROSIDL_GENERATOR_C_PUBLIC_safety_manager
const rosidl_type_hash_t *
safety_manager__msg__VehicleState__get_type_hash(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_type_hash_t hash = {1, {
      0x6a, 0x0a, 0xfd, 0xc6, 0xd7, 0xdb, 0x08, 0x73,
      0xb2, 0x3e, 0x74, 0xc7, 0x2d, 0xca, 0xe8, 0x83,
      0x78, 0x34, 0x9f, 0x83, 0xac, 0x77, 0xb6, 0xb5,
      0x32, 0x99, 0x51, 0x32, 0xf0, 0xea, 0x9c, 0xe7,
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

static char safety_manager__msg__VehicleState__TYPE_NAME[] = "safety_manager/msg/VehicleState";
static char builtin_interfaces__msg__Time__TYPE_NAME[] = "builtin_interfaces/msg/Time";

// Define type names, field names, and default values
static char safety_manager__msg__VehicleState__FIELD_NAME__stamp[] = "stamp";
static char safety_manager__msg__VehicleState__FIELD_NAME__state[] = "state";
static char safety_manager__msg__VehicleState__FIELD_NAME__reason[] = "reason";

static rosidl_runtime_c__type_description__Field safety_manager__msg__VehicleState__FIELDS[] = {
  {
    {safety_manager__msg__VehicleState__FIELD_NAME__stamp, 5, 5},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_NESTED_TYPE,
      0,
      0,
      {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__VehicleState__FIELD_NAME__state, 5, 5},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_UINT8,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
  {
    {safety_manager__msg__VehicleState__FIELD_NAME__reason, 6, 6},
    {
      rosidl_runtime_c__type_description__FieldType__FIELD_TYPE_STRING,
      0,
      0,
      {NULL, 0, 0},
    },
    {NULL, 0, 0},
  },
};

static rosidl_runtime_c__type_description__IndividualTypeDescription safety_manager__msg__VehicleState__REFERENCED_TYPE_DESCRIPTIONS[] = {
  {
    {builtin_interfaces__msg__Time__TYPE_NAME, 27, 27},
    {NULL, 0, 0},
  },
};

const rosidl_runtime_c__type_description__TypeDescription *
safety_manager__msg__VehicleState__get_type_description(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static bool constructed = false;
  static const rosidl_runtime_c__type_description__TypeDescription description = {
    {
      {safety_manager__msg__VehicleState__TYPE_NAME, 31, 31},
      {safety_manager__msg__VehicleState__FIELDS, 3, 3},
    },
    {safety_manager__msg__VehicleState__REFERENCED_TYPE_DESCRIPTIONS, 1, 1},
  };
  if (!constructed) {
    assert(0 == memcmp(&builtin_interfaces__msg__Time__EXPECTED_HASH, builtin_interfaces__msg__Time__get_type_hash(NULL), sizeof(rosidl_type_hash_t)));
    description.referenced_type_descriptions.data[0].fields = builtin_interfaces__msg__Time__get_type_description(NULL)->type_description.fields;
    constructed = true;
  }
  return &description;
}

static char toplevel_type_raw_source[] =
  "# Arbitrated vehicle safety state. safety_manager is the only node allowed\n"
  "# to publish this; all actuation nodes must gate their outputs on it.\n"
  "\n"
  "builtin_interfaces/Time stamp\n"
  "\n"
  "uint8 NORMAL         = 0\n"
  "uint8 AVOID          = 1\n"
  "uint8 BRAKE          = 2\n"
  "uint8 REVERSE         = 3\n"
  "uint8 RECOVER         = 4\n"
  "uint8 EMERGENCY_STOP  = 5\n"
  "\n"
  "uint8 state\n"
  "string reason";

static char msg_encoding[] = "msg";

// Define all individual source functions

const rosidl_runtime_c__type_description__TypeSource *
safety_manager__msg__VehicleState__get_individual_type_description_source(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static const rosidl_runtime_c__type_description__TypeSource source = {
    {safety_manager__msg__VehicleState__TYPE_NAME, 31, 31},
    {msg_encoding, 3, 3},
    {toplevel_type_raw_source, 357, 357},
  };
  return &source;
}

const rosidl_runtime_c__type_description__TypeSource__Sequence *
safety_manager__msg__VehicleState__get_type_description_sources(
  const rosidl_message_type_support_t * type_support)
{
  (void)type_support;
  static rosidl_runtime_c__type_description__TypeSource sources[2];
  static const rosidl_runtime_c__type_description__TypeSource__Sequence source_sequence = {sources, 2, 2};
  static bool constructed = false;
  if (!constructed) {
    sources[0] = *safety_manager__msg__VehicleState__get_individual_type_description_source(NULL),
    sources[1] = *builtin_interfaces__msg__Time__get_individual_type_description_source(NULL);
    constructed = true;
  }
  return &source_sequence;
}
