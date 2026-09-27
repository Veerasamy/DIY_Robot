// Selects a concrete ISteeringController at node startup based on the
// `controller_type` ROS parameter, so pure_pursuit_node can be repointed at
// Stanley or MPC for A/B comparison without any node-wiring changes -- the
// entire point of the ISteeringController interface (ARCHITECTURE.md
// section 4).
#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "controller/mpc_controller.hpp"
#include "controller/pure_pursuit_controller.hpp"
#include "controller/stanley_controller.hpp"
#include "controller/steering_controller_interface.hpp"

namespace controller
{

struct ControllerFactoryParams
{
  std::string type{"pure_pursuit"};  // "pure_pursuit" | "stanley" | "mpc"
  PurePursuitParams pure_pursuit;
  StanleyParams stanley;
  MpcParams mpc;
};

inline std::unique_ptr<ISteeringController> createSteeringController(
  const ControllerFactoryParams & params)
{
  if (params.type == "pure_pursuit") {
    return std::make_unique<PurePursuitController>(params.pure_pursuit);
  }
  if (params.type == "stanley") {
    return std::make_unique<StanleyController>(params.stanley);
  }
  if (params.type == "mpc") {
    return std::make_unique<MpcController>(params.mpc);
  }
  throw std::invalid_argument(
    "Unknown controller_type '" + params.type + "' (expected pure_pursuit|stanley|mpc)");
}

}  // namespace controller
