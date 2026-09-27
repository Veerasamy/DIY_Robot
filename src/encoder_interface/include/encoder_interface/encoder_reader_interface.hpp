// Hardware-agnostic wheel-encoder tick source. Concrete backends (GPIO
// quadrature decode on the Jetson, or a future ESP32-relayed backend) are
// selected in encoder_node.cpp — mirrors the `ISteeringController` pattern
// in the `controller` package so the node wiring never changes when the
// backend does.
#pragma once

#include <cstdint>

namespace encoder_interface
{

class IEncoderReader
{
public:
  virtual ~IEncoderReader() = default;

  // Opens/starts the hardware backend. Returns false on failure (node logs
  // and keeps publishing zero velocity rather than crashing, matching the
  // fail-safe posture used elsewhere in this stack).
  virtual bool start() = 0;
  virtual void stop() = 0;

  // Signed cumulative tick count since start() (+ = forward rotation).
  virtual int64_t totalTicks() const = 0;
};

}  // namespace encoder_interface
