// Quadrature decoder for the SS49E-style dual-Hall wheel encoder (forward +
// backward detection channels A/B), read via libgpiod line-edge events on
// the Jetson Orin Nano's 40-pin GPIO header.
//
// Decode scheme: a dedicated thread blocks on edge events for channel A
// only; each A edge is classified forward/backward by sampling channel B's
// instantaneous level at that edge (the standard 2x quadrature decode — 1x
// per full magnet-pole pair doubled to 2x by counting both A edges, not the
// full 4x decode that would also count B edges). This keeps the polling
// thread simple (one blocking wait, not two) while still giving direction
// discrimination, which a single-Hall-sensor tachometer cannot provide.
//
// libgpiod (not the deprecated sysfs GPIO interface) is required — install
// with `sudo apt install gpiod libgpiod-dev`. If the library is not present
// at build time this class is compiled out (see CMakeLists.txt) and
// `encoder_node` falls back to `NullEncoderReader`, publishing zero velocity
// with a clear log message instead of failing to build/run.
#pragma once

#include <atomic>
#include <string>
#include <thread>

#include "encoder_interface/encoder_reader_interface.hpp"

struct gpiod_chip;
struct gpiod_line;

namespace encoder_interface
{

struct GpioQuadratureParams
{
  std::string gpio_chip{"/dev/gpiochip0"};
  unsigned int pin_a{0};  // BCM/offset numbering per `gpioinfo` — MEASURE ON YOUR WIRING
  unsigned int pin_b{0};
  std::string consumer_label{"encoder_interface"};
};

class GpioQuadratureReader : public IEncoderReader
{
public:
  explicit GpioQuadratureReader(const GpioQuadratureParams & params) : params_(params) {}
  ~GpioQuadratureReader() override;

  bool start() override;
  void stop() override;
  int64_t totalTicks() const override {return ticks_.load();}

private:
  void pollLoop();

  GpioQuadratureParams params_;
  gpiod_chip * chip_{nullptr};
  gpiod_line * line_a_{nullptr};
  gpiod_line * line_b_{nullptr};
  std::thread poll_thread_;
  std::atomic<bool> running_{false};
  std::atomic<int64_t> ticks_{0};
};

// Fail-safe stand-in used when libgpiod isn't available at build time, or
// when the GPIO lines fail to open at runtime (e.g. wrong pin numbers,
// permissions). Always reports zero ticks; sensor_fusion's EKF still runs
// on ZED VO + IMU alone (encoder is one of three fused measurements, not a
// hard dependency — see ARCHITECTURE.md section 6).
class NullEncoderReader : public IEncoderReader
{
public:
  bool start() override {return true;}
  void stop() override {}
  int64_t totalTicks() const override {return 0;}
};

}  // namespace encoder_interface
