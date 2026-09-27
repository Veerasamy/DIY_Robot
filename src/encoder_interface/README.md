# encoder_interface

GPIO quadrature decoder for the SCRT10's dual-Hall (SS49E-style, forward +
backward channels) wheel encoder, publishing the two topics `sensor_fusion`'s
EKF and the rest of the stack expect (ARCHITECTURE.md topic contract):

- `/encoder/ticks` (`std_msgs/Int32`) — signed cumulative tick count
- `/encoder/velocity_mps` (`std_msgs/Float32`) — EMA-filtered forward speed,
  consumed directly by `sensor_fusion`'s EKF measurement update

## Hardware backend

Reads channel A/B via `libgpiod` line-edge events on the Jetson's 40-pin
header (not the deprecated sysfs GPIO interface). Requires:

```bash
sudo apt install libgpiod-dev gpiod
```

If `libgpiod-dev` isn't present at build time, `encoder_node` still builds
and runs but publishes zero ticks/velocity via `NullEncoderReader` with a
clear warning log — `sensor_fusion`'s EKF degrades gracefully to ZED VO + IMU
only in that case (encoder is one of three fused measurements, not a hard
dependency).

## Action required before racing

- **`gpio_pin_a` / `gpio_pin_b`** — placeholders (0/0). Wire the encoder's A/B
  outputs to two free Jetson GPIO header pins, confirm the *line offset*
  (not the physical pin number) with `gpioinfo`, and set both parameters.
- **`pulses_per_revolution`** — placeholder (40.0). Count the SS49E target's
  actual pole pairs per wheel revolution on the assembled SCRT10.
- **`wheel_circumference_m`** — placeholder (0.2m). Measure the actual tire
  rolling circumference (not the nominal diameter — tire compression under
  load changes this).

## Decode scheme

2x quadrature decode: a dedicated thread blocks on channel A edge events
only; each edge's direction is classified by sampling channel B's level at
that instant. This is not a full 4x decode (which would also count B edges)
but is sufficient resolution for velocity estimation while keeping the
event-polling thread simple (see `gpio_quadrature_reader.cpp` for the exact
rising/falling logic).

## Topics

- Publishes: `/encoder/ticks` (`std_msgs/Int32`), `/encoder/velocity_mps`
  (`std_msgs/Float32`)
