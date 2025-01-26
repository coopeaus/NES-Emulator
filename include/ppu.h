#pragma once
#include <cstdint>
#include <array>

using u8=std::uint8_t;
using u16=std::uint16_t;


class PPU
{
public:
  PPU();

  [[nodiscard]] u8 HandleCpuRead(u16 address);
  void HandleCpuWrite(u16 address, u8 data);

private:
  std::array<u8, 8> _ppuRegisters{};
};