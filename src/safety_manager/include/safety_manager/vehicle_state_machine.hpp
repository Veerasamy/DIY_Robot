// Vehicle safety state machine (Section G of the project brief).
//
//               estop_pressed (any source)
//        ┌───────────────────────────────────────────┐
//        │                                            ▼
//  ┌───────────┐  obstacle_close   ┌───────┐   collision_imminent   ┌───────┐
//  │  NORMAL   │ ─────────────────▶│ AVOID │ ──────────────────────▶│ BRAKE │
//  └───────────┘◀───────────────── └───────┘                        └───┬───┘
//        ▲          avoided_ok                                          │ stopped
//        │                                                               ▼
//        │                                                        ┌───────────┐
//        │                     recovered                          │  REVERSE  │
//        └───────────────────────────────────────────────────────  └─────┬─────┘
//                                                                          │ clear_path
//                                                                          ▼
//                                                                   ┌───────────┐
//                                                                   │  RECOVER  │
//                                                                   └─────┬─────┘
//                                                                          │ recovered
//                                                                          └──▶ NORMAL
//
// EMERGENCY_STOP can be entered from *any* state and can only be exited by
// an explicit operator clear (never automatically), matching the
// competition's E-Stop semantics: "E-stop pressed -> circuit open",
// released only clears the *input*, the vehicle software still requires a
// deliberate re-arm before resuming motion.
#pragma once

#include <cstdint>
#include <string>

namespace safety_manager
{

enum class VehicleState : uint8_t
{
  kNormal = 0,
  kAvoid = 1,
  kBrake = 2,
  kReverse = 3,
  kRecover = 4,
  kEmergencyStop = 5,
};

// Inputs sampled once per control cycle (~50 Hz) and fed to update().
struct StateInputs
{
  bool estop_pressed{false};   // from safety_manager's UART link (or its loss)
  bool obstacle_close{false};  // from obstacle_detection
  bool collision_imminent{false};  // from obstacle_detection TTC estimate
  bool vehicle_stopped{false};  // from sensor_fusion (|v| < epsilon)
  bool path_clear{false};      // from obstacle_detection after reversing
  bool recovered{false};       // from obstacle_detection / planner
  bool operator_clear_requested{false};  // explicit re-arm (never automatic)
};

class VehicleStateMachine
{
public:
  VehicleState update(const StateInputs & in);

  VehicleState state() const {return state_;}
  const std::string & reason() const {return reason_;}

  static const char * toString(VehicleState s);

private:
  void transition(VehicleState next, std::string why);

  VehicleState state_{VehicleState::kNormal};
  std::string reason_{"init"};
};

}  // namespace safety_manager
