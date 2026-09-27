#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to safety_manager__msg__EStopStatus
/// Decoded status of the physical E-Stop radio link, as reported by the
/// ESP32 over UART. Published at the UART packet arrival rate (>= 5 Hz).

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct EStopStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::Time,

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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::EStopStatus::default())
  }
}

impl rosidl_runtime_rs::Message for EStopStatus {
  type RmwMsg = super::msg::rmw::EStopStatus;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Owned(msg.stamp)).into_owned(),
        estop_pressed: msg.estop_pressed,
        link_ok: msg.link_ok,
        radio_ok: msg.radio_ok,
        heartbeat_seq: msg.heartbeat_seq,
        battery_millivolts: msg.battery_millivolts,
        radio_rssi_dbm: msg.radio_rssi_dbm,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Borrowed(&msg.stamp)).into_owned(),
      estop_pressed: msg.estop_pressed,
      link_ok: msg.link_ok,
      radio_ok: msg.radio_ok,
      heartbeat_seq: msg.heartbeat_seq,
      battery_millivolts: msg.battery_millivolts,
      radio_rssi_dbm: msg.radio_rssi_dbm,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      stamp: builtin_interfaces::msg::Time::from_rmw_message(msg.stamp),
      estop_pressed: msg.estop_pressed,
      link_ok: msg.link_ok,
      radio_ok: msg.radio_ok,
      heartbeat_seq: msg.heartbeat_seq,
      battery_millivolts: msg.battery_millivolts,
      radio_rssi_dbm: msg.radio_rssi_dbm,
    }
  }
}


// Corresponds to safety_manager__msg__VehicleState
/// Arbitrated vehicle safety state. safety_manager is the only node allowed
/// to publish this; all actuation nodes must gate their outputs on it.

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct VehicleState {

    // This member is not documented.
    #[allow(missing_docs)]
    pub stamp: builtin_interfaces::msg::Time,


    // This member is not documented.
    #[allow(missing_docs)]
    pub state: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub reason: std::string::String,

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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::VehicleState::default())
  }
}

impl rosidl_runtime_rs::Message for VehicleState {
  type RmwMsg = super::msg::rmw::VehicleState;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Owned(msg.stamp)).into_owned(),
        state: msg.state,
        reason: msg.reason.as_str().into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        stamp: builtin_interfaces::msg::Time::into_rmw_message(std::borrow::Cow::Borrowed(&msg.stamp)).into_owned(),
      state: msg.state,
        reason: msg.reason.as_str().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      stamp: builtin_interfaces::msg::Time::from_rmw_message(msg.stamp),
      state: msg.state,
      reason: msg.reason.to_string(),
    }
  }
}


