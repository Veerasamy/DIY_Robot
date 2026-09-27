#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "safety_manager__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__safety_manager__msg__EStopStatus() -> *const std::ffi::c_void;
}

#[link(name = "safety_manager__rosidl_generator_c")]
extern "C" {
    fn safety_manager__msg__EStopStatus__init(msg: *mut EStopStatus) -> bool;
    fn safety_manager__msg__EStopStatus__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<EStopStatus>, size: usize) -> bool;
    fn safety_manager__msg__EStopStatus__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<EStopStatus>);
    fn safety_manager__msg__EStopStatus__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<EStopStatus>, out_seq: *mut rosidl_runtime_rs::Sequence<EStopStatus>) -> bool;
}

// Corresponds to safety_manager__msg__EStopStatus
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// Decoded status of the physical E-Stop radio link, as reported by the
/// ESP32 over UART. Published at the UART packet arrival rate (>= 5 Hz).

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct EStopStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::rmw::Time,

    /// True when the course E-Stop relay is OPEN (button pressed) as read back
    /// on RJ45 pins 5-8 by the team radio and relayed to the robot.
    pub estop_pressed: bool,

    /// True while heartbeat packets are arriving from the ESP32 within the
    /// configured timeout. False is treated as an implicit E-Stop (fail-safe).
    pub link_ok: bool,

    /// True while the team-radio <-> robot-radio RF link itself is healthy
    /// (distinct from the Jetson<->ESP32 UART link captured by link_ok).
    pub radio_ok: bool,


    // This member is not documented.
    #[allow(missing_docs)]
    pub heartbeat_seq: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub battery_millivolts: u16,


    // This member is not documented.
    #[allow(missing_docs)]
    pub radio_rssi_dbm: i8,

}



impl Default for EStopStatus {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !safety_manager__msg__EStopStatus__init(&mut msg as *mut _) {
        panic!("Call to safety_manager__msg__EStopStatus__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for EStopStatus {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__EStopStatus__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__EStopStatus__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__EStopStatus__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for EStopStatus {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for EStopStatus where Self: Sized {
  const TYPE_NAME: &'static str = "safety_manager/msg/EStopStatus";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__safety_manager__msg__EStopStatus() }
  }
}


#[link(name = "safety_manager__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__safety_manager__msg__VehicleState() -> *const std::ffi::c_void;
}

#[link(name = "safety_manager__rosidl_generator_c")]
extern "C" {
    fn safety_manager__msg__VehicleState__init(msg: *mut VehicleState) -> bool;
    fn safety_manager__msg__VehicleState__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<VehicleState>, size: usize) -> bool;
    fn safety_manager__msg__VehicleState__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<VehicleState>);
    fn safety_manager__msg__VehicleState__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<VehicleState>, out_seq: *mut rosidl_runtime_rs::Sequence<VehicleState>) -> bool;
}

// Corresponds to safety_manager__msg__VehicleState
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// Arbitrated vehicle safety state. safety_manager is the only node allowed
/// to publish this; all actuation nodes must gate their outputs on it.

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct VehicleState {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::rmw::Time,


    // This member is not documented.
    #[allow(missing_docs)]
    pub state: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub reason: rosidl_runtime_rs::String,

}

impl VehicleState {

    // This constant is not documented.
    #[allow(missing_docs)]
    pub const NORMAL: u8 = 0;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const AVOID: u8 = 1;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const BRAKE: u8 = 2;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const REVERSE: u8 = 3;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const RECOVER: u8 = 4;


    // This constant is not documented.
    #[allow(missing_docs)]
    pub const EMERGENCY_STOP: u8 = 5;

}


impl Default for VehicleState {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !safety_manager__msg__VehicleState__init(&mut msg as *mut _) {
        panic!("Call to safety_manager__msg__VehicleState__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for VehicleState {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__VehicleState__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__VehicleState__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { safety_manager__msg__VehicleState__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for VehicleState {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for VehicleState where Self: Sized {
  const TYPE_NAME: &'static str = "safety_manager/msg/VehicleState";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__safety_manager__msg__VehicleState() }
  }
}


