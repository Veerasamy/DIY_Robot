// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from safety_manager:msg/EStopStatus.idl
// generated code does not contain a copyright notice
#include "safety_manager/msg/detail/e_stop_status__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `stamp`
#include "builtin_interfaces/msg/detail/time__functions.h"

bool
safety_manager__msg__EStopStatus__init(safety_manager__msg__EStopStatus * msg)
{
  if (!msg) {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__init(&msg->stamp)) {
    safety_manager__msg__EStopStatus__fini(msg);
    return false;
  }
  // estop_pressed
  // link_ok
  // radio_ok
  // heartbeat_seq
  // battery_millivolts
  // radio_rssi_dbm
  return true;
}

void
safety_manager__msg__EStopStatus__fini(safety_manager__msg__EStopStatus * msg)
{
  if (!msg) {
    return;
  }
  // stamp
  builtin_interfaces__msg__Time__fini(&msg->stamp);
  // estop_pressed
  // link_ok
  // radio_ok
  // heartbeat_seq
  // battery_millivolts
  // radio_rssi_dbm
}

bool
safety_manager__msg__EStopStatus__are_equal(const safety_manager__msg__EStopStatus * lhs, const safety_manager__msg__EStopStatus * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__are_equal(
      &(lhs->stamp), &(rhs->stamp)))
  {
    return false;
  }
  // estop_pressed
  if (lhs->estop_pressed != rhs->estop_pressed) {
    return false;
  }
  // link_ok
  if (lhs->link_ok != rhs->link_ok) {
    return false;
  }
  // radio_ok
  if (lhs->radio_ok != rhs->radio_ok) {
    return false;
  }
  // heartbeat_seq
  if (lhs->heartbeat_seq != rhs->heartbeat_seq) {
    return false;
  }
  // battery_millivolts
  if (lhs->battery_millivolts != rhs->battery_millivolts) {
    return false;
  }
  // radio_rssi_dbm
  if (lhs->radio_rssi_dbm != rhs->radio_rssi_dbm) {
    return false;
  }
  return true;
}

bool
safety_manager__msg__EStopStatus__copy(
  const safety_manager__msg__EStopStatus * input,
  safety_manager__msg__EStopStatus * output)
{
  if (!input || !output) {
    return false;
  }
  // stamp
  if (!builtin_interfaces__msg__Time__copy(
      &(input->stamp), &(output->stamp)))
  {
    return false;
  }
  // estop_pressed
  output->estop_pressed = input->estop_pressed;
  // link_ok
  output->link_ok = input->link_ok;
  // radio_ok
  output->radio_ok = input->radio_ok;
  // heartbeat_seq
  output->heartbeat_seq = input->heartbeat_seq;
  // battery_millivolts
  output->battery_millivolts = input->battery_millivolts;
  // radio_rssi_dbm
  output->radio_rssi_dbm = input->radio_rssi_dbm;
  return true;
}

safety_manager__msg__EStopStatus *
safety_manager__msg__EStopStatus__create(void)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  safety_manager__msg__EStopStatus * msg = (safety_manager__msg__EStopStatus *)allocator.allocate(sizeof(safety_manager__msg__EStopStatus), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(safety_manager__msg__EStopStatus));
  bool success = safety_manager__msg__EStopStatus__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
safety_manager__msg__EStopStatus__destroy(safety_manager__msg__EStopStatus * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    safety_manager__msg__EStopStatus__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
safety_manager__msg__EStopStatus__Sequence__init(safety_manager__msg__EStopStatus__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  safety_manager__msg__EStopStatus * data = NULL;

  if (size) {
    if (size > SIZE_MAX / sizeof(safety_manager__msg__EStopStatus)) {
      return false;
    }
    data = (safety_manager__msg__EStopStatus *)allocator.zero_allocate(size, sizeof(safety_manager__msg__EStopStatus), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = safety_manager__msg__EStopStatus__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        safety_manager__msg__EStopStatus__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
safety_manager__msg__EStopStatus__Sequence__fini(safety_manager__msg__EStopStatus__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      safety_manager__msg__EStopStatus__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

safety_manager__msg__EStopStatus__Sequence *
safety_manager__msg__EStopStatus__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  safety_manager__msg__EStopStatus__Sequence * array = (safety_manager__msg__EStopStatus__Sequence *)allocator.allocate(sizeof(safety_manager__msg__EStopStatus__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = safety_manager__msg__EStopStatus__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
safety_manager__msg__EStopStatus__Sequence__destroy(safety_manager__msg__EStopStatus__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    safety_manager__msg__EStopStatus__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
safety_manager__msg__EStopStatus__Sequence__are_equal(const safety_manager__msg__EStopStatus__Sequence * lhs, const safety_manager__msg__EStopStatus__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!safety_manager__msg__EStopStatus__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
safety_manager__msg__EStopStatus__Sequence__copy(
  const safety_manager__msg__EStopStatus__Sequence * input,
  safety_manager__msg__EStopStatus__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    if (input->size > SIZE_MAX / sizeof(safety_manager__msg__EStopStatus)) {
      return false;
    }
    const size_t allocation_size =
      input->size * sizeof(safety_manager__msg__EStopStatus);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    safety_manager__msg__EStopStatus * data =
      (safety_manager__msg__EStopStatus *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!safety_manager__msg__EStopStatus__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          safety_manager__msg__EStopStatus__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!safety_manager__msg__EStopStatus__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
