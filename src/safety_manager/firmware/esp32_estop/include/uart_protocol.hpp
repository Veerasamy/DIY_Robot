// SYNCED COPY of safety_manager/common/uart_protocol.hpp for the PlatformIO
// build (PlatformIO auto-includes this project's own include/ directory and
// cannot reach into the colcon package tree). Keep byte-for-byte identical
// to ../../../common/uart_protocol.hpp whenever the protocol changes.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace safety_manager::uart
{

constexpr uint8_t kStartByte = 0xAA;
constexpr uint8_t kEndByte = 0x55;

constexpr uint8_t kPacketTypeStatus = 0x01;
constexpr size_t kStatusPacketLen = 10;

struct StatusPacket
{
  bool estop_pressed{false};
  bool radio_ok{false};
  uint8_t heartbeat_seq{0};
  uint16_t battery_millivolts{0};
  int8_t radio_rssi_dbm{0};
};

constexpr uint8_t kPacketTypeCommand = 0x10;
constexpr size_t kCommandPacketLen = 6;

constexpr uint8_t kCmdWatchdogKick = 0x00;
constexpr uint8_t kCmdSoftwareEStop = 0x01;
constexpr uint8_t kCmdClearSoftwareEStop = 0x02;

inline uint8_t crc8(const uint8_t * data, size_t len)
{
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; ++i) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; ++bit) {
      if (crc & 0x01) {
        crc = static_cast<uint8_t>((crc >> 1) ^ 0x8C);
      } else {
        crc = static_cast<uint8_t>(crc >> 1);
      }
    }
  }
  return crc;
}

inline std::array<uint8_t, kStatusPacketLen> encodeStatus(const StatusPacket & p)
{
  std::array<uint8_t, kStatusPacketLen> buf{};
  buf[0] = kStartByte;
  buf[1] = kPacketTypeStatus;
  buf[2] = p.estop_pressed ? 1 : 0;
  buf[3] = p.radio_ok ? 1 : 0;
  buf[4] = p.heartbeat_seq;
  buf[5] = static_cast<uint8_t>(p.battery_millivolts & 0xFF);
  buf[6] = static_cast<uint8_t>((p.battery_millivolts >> 8) & 0xFF);
  buf[7] = static_cast<uint8_t>(p.radio_rssi_dbm);
  buf[8] = crc8(buf.data(), 8);
  buf[9] = kEndByte;
  return buf;
}

inline bool decodeStatus(const uint8_t * buf, size_t len, StatusPacket & out)
{
  if (len != kStatusPacketLen) {return false;}
  if (buf[0] != kStartByte || buf[1] != kPacketTypeStatus) {return false;}
  if (buf[9] != kEndByte) {return false;}
  if (crc8(buf, 8) != buf[8]) {return false;}

  out.estop_pressed = buf[2] != 0;
  out.radio_ok = buf[3] != 0;
  out.heartbeat_seq = buf[4];
  out.battery_millivolts = static_cast<uint16_t>(buf[5] | (buf[6] << 8));
  out.radio_rssi_dbm = static_cast<int8_t>(buf[7]);
  return true;
}

inline std::array<uint8_t, kCommandPacketLen> encodeCommand(uint8_t command, uint8_t sequence)
{
  std::array<uint8_t, kCommandPacketLen> buf{};
  buf[0] = kStartByte;
  buf[1] = kPacketTypeCommand;
  buf[2] = command;
  buf[3] = sequence;
  buf[4] = crc8(buf.data(), 4);
  buf[5] = kEndByte;
  return buf;
}

}  // namespace safety_manager::uart
