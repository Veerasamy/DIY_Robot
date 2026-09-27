#include "safety_manager/vehicle_state_machine.hpp"

namespace safety_manager
{

VehicleState VehicleStateMachine::update(const StateInputs & in)
{
  // EMERGENCY_STOP pre-empts every other transition and can only be left
  // by an explicit operator clear — never inferred from sensor state.
  if (in.estop_pressed) {
    transition(VehicleState::kEmergencyStop, "estop_pressed");
    return state_;
  }
  if (state_ == VehicleState::kEmergencyStop) {
    if (in.operator_clear_requested) {
      transition(VehicleState::kNormal, "operator_clear_requested");
    }
    return state_;
  }

  switch (state_) {
    case VehicleState::kNormal:
      if (in.collision_imminent) {
        transition(VehicleState::kBrake, "collision_imminent");
      } else if (in.obstacle_close) {
        transition(VehicleState::kAvoid, "obstacle_close");
      }
      break;

    case VehicleState::kAvoid:
      if (in.collision_imminent) {
        transition(VehicleState::kBrake, "collision_imminent");
      } else if (!in.obstacle_close) {
        transition(VehicleState::kNormal, "avoided_ok");
      }
      break;

    case VehicleState::kBrake:
      if (in.vehicle_stopped) {
        transition(VehicleState::kReverse, "stopped");
      }
      break;

    case VehicleState::kReverse:
      if (in.path_clear) {
        transition(VehicleState::kRecover, "clear_path");
      }
      break;

    case VehicleState::kRecover:
      if (in.recovered) {
        transition(VehicleState::kNormal, "recovered");
      }
      break;

    case VehicleState::kEmergencyStop:
      // handled above
      break;
  }
  return state_;
}

void VehicleStateMachine::transition(VehicleState next, std::string why)
{
  state_ = next;
  reason_ = std::move(why);
}

const char * VehicleStateMachine::toString(VehicleState s)
{
  switch (s) {
    case VehicleState::kNormal: return "NORMAL";
    case VehicleState::kAvoid: return "AVOID";
    case VehicleState::kBrake: return "BRAKE";
    case VehicleState::kReverse: return "REVERSE";
    case VehicleState::kRecover: return "RECOVER";
    case VehicleState::kEmergencyStop: return "EMERGENCY_STOP";
  }
  return "UNKNOWN";
}

}  // namespace safety_manager
