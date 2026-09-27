#include "encoder_interface/gpio_quadrature_reader.hpp"

#include <gpiod.h>

#include <cstdio>

namespace encoder_interface
{

GpioQuadratureReader::~GpioQuadratureReader()
{
  stop();
}

bool GpioQuadratureReader::start()
{
  chip_ = gpiod_chip_open(params_.gpio_chip.c_str());
  if (!chip_) {
    std::fprintf(
      stderr, "encoder_interface: failed to open %s (check permissions / pin config)\n",
      params_.gpio_chip.c_str());
    return false;
  }

  line_a_ = gpiod_chip_get_line(chip_, params_.pin_a);
  line_b_ = gpiod_chip_get_line(chip_, params_.pin_b);
  if (!line_a_ || !line_b_) {
    std::fprintf(stderr, "encoder_interface: failed to get GPIO lines %u/%u\n",
      params_.pin_a, params_.pin_b);
    stop();
    return false;
  }

  if (gpiod_line_request_both_edges_events(line_a_, params_.consumer_label.c_str()) != 0) {
    std::fprintf(stderr, "encoder_interface: failed to request edge events on channel A\n");
    stop();
    return false;
  }
  if (gpiod_line_request_input(line_b_, params_.consumer_label.c_str()) != 0) {
    std::fprintf(stderr, "encoder_interface: failed to request channel B as input\n");
    stop();
    return false;
  }

  running_ = true;
  poll_thread_ = std::thread(&GpioQuadratureReader::pollLoop, this);
  return true;
}

void GpioQuadratureReader::stop()
{
  running_ = false;
  if (poll_thread_.joinable()) {
    poll_thread_.join();
  }
  if (line_a_) {gpiod_line_release(line_a_); line_a_ = nullptr;}
  if (line_b_) {gpiod_line_release(line_b_); line_b_ = nullptr;}
  if (chip_) {gpiod_chip_close(chip_); chip_ = nullptr;}
}

void GpioQuadratureReader::pollLoop()
{
  const struct timespec timeout{0, 100'000'000};  // 100ms — lets running_ be re-checked promptly
  while (running_.load()) {
    int rc = gpiod_line_event_wait(line_a_, &timeout);
    if (rc <= 0) {continue;}  // 0 = timeout (re-check running_), <0 = error, retry

    struct gpiod_line_event event;
    if (gpiod_line_event_read(line_a_, &event) != 0) {continue;}

    // Standard 2x quadrature decode: on a rising edge of A, B-high means A
    // leads B (forward); on a falling edge of A, the relationship inverts.
    int b_level = gpiod_line_get_value(line_b_);
    bool rising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
    bool forward = rising ? (b_level == 0) : (b_level != 0);

    ticks_.fetch_add(forward ? 1 : -1);
  }
}

}  // namespace encoder_interface
