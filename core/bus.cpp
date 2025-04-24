#include "bus.h"
#include "cartridge.h"
#include "global-types.h"
#include <iostream>
#include "utils.h"

// Constructor to initialize the bus with a flat memory model
Bus::Bus() : cpu( this ), ppu( this ), cartridge( this ), apu( this )
{
}

/*
################################
||          CPU Read          ||
################################
*/
u8 Bus::Read( const u16 address, bool debugMode )
{
  if ( _useFlatMemory ) {
    return _flatMemory.at( address );
  }

  // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    return _ram.at( address & 0x07FF );
  }

  // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
  if ( address >= 0x2000 && address <= 0x3FFF ) {
    // ppu read will go here. For now, return from temp private member of bus
    const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
    return ppu.CpuRead( ppuRegister, debugMode );
  }

  // APU
  if ( utils::between( address, 0x4000, 0x4013 ) || address == 0x4015 ) {
    return apu.HandleCpuRead( address );
  }

  // Controller read
  if ( address >= 0x4016 && address <= 0x4017 ) { // FIX: 0x4017 for controllers or for APU?
    auto data = ( controllerState[address & 0x0001] & 0x80 ) > 0;
    controllerState[address & 0x0001] <<= 1;
    return data;
  }

  // 4020 and up is cartridge territory
  if ( address >= 0x4020 && address <= 0xFFFF ) {
    return cartridge.Read( address );
  }

  // Unhandled address ranges return open bus value
  std::cout << "Unhandled read from address: " << std::hex << address << "\n";
  return 0xFF;
}

/*
################################
||          CPU Write         ||
################################
*/
void Bus::Write( const u16 address, const u8 data )
{

  if ( _useFlatMemory ) {
    _flatMemory.at( address ) = data;
    return;
  }

  // System RAM: 0x0000 - 0x1FFF (mirrored every 2KB)
  if ( address >= 0x0000 && address <= 0x1FFF ) {
    _ram.at( address & 0x07FF ) = data;
    return;
  }

  // PPU Registers: 0x2000 - 0x3FFF (mirrored every 8 bytes)
  if ( address >= 0x2000 && address <= 0x3FFF ) {
    const u16 ppuRegister = 0x2000 + ( address & 0x0007 );
    ppu.CpuWrite( ppuRegister, data );
    return;
  }

  // PPU DMA: 0x4014
  if ( address == 0x4014 ) {
    dmaInProgress = true;
    dmaAddr = data << 8;
    dmaOffset = 0;
    return;
  }

  // Controller input
  if ( address >= 0x4016 && address <= 0x4017 ) { // FIX: 0x4017 for controllers or APU?
    controllerState[address & 0x0001] = controller[address & 0x0001];
    return;
  }

  // APU
  if ( utils::between( address, 0x4000, 0x4013 ) || address == 0x4015 ) {
    apu.HandleCpuWrite( address, data );
    return;
  }

  // 4020 and up is cartridge territory
  if ( address >= 0x4020 && address <= 0xFFFF ) {
    cartridge.Write( address, data );
    return;
  }
  // Unhandled address ranges
  // Optionally log a warning or ignore
  std::cout << "Unhandled write to address: " << std::hex << address << "\n";
}

void Bus::ProcessDma()
{
  const u64 cycle = cpu.GetCycles();

  u8 const oamAddr = ppu.oamAddr;
  // Wait first read is on an odd cycle, wait it out.
  if ( dmaOffset == 0 && cycle % 2 == 1 ) {
    cpu.Tick();
    return;
  }

  // Read into OAM on even, load next byte on odd
  if ( cycle % 2 == 0 ) {
    auto data = Read( dmaAddr + dmaOffset );
    cpu.Tick();
    ppu.oam.data.at( ( oamAddr + dmaOffset ) & 0xFF ) = data;
    dmaOffset++;
  } else {
    dmaInProgress = dmaOffset < 256;
    cpu.Tick();
  }
}

void Bus::Clock()
{
  if ( dmaInProgress ) {
    ProcessDma();
  } else {
    cpu.DecodeExecute();
  }

  if ( ppu.nmiReady ) {
    ppu.nmiReady = false;
    cpu.NMI();
  }
}

/*
################################
||        Debug Methods       ||
################################
*/
[[nodiscard]] bool Bus::IsTestMode() const
{
  return _useFlatMemory;
}
void Bus::DebugReset()
{
  cpu.SetCycles( 0 );
  cpu.Reset();
  ppu.Reset();
}

bool Bus::LoadStateFromJson(const nlohmann::json &jsonData, const std::string &state) {
  // CPU
  cpu.SetProgramCounter(jsonData[state]["cpu"]["pc"]);
  cpu.SetAccumulator(jsonData[state]["cpu"]["a"]);
  cpu.SetXRegister(jsonData[state]["cpu"]["x"]);
  cpu.SetYRegister(jsonData[state]["cpu"]["y"]);
  cpu.SetStackPointer(jsonData[state]["cpu"]["s"]);
  cpu.SetStatusRegister(jsonData[state]["cpu"]["p"]);
  cpu.SetCycles(jsonData[state]["cpu"]["cycles"]);

  //load CPU
  for (const auto &entry : jsonData[state]["cpu"]["ram"]) {
    u16 addr = entry[0];
    u8 val = entry[1];
    cpu.Write(addr, val);
  }

  //Cartridge
  for (const auto &entry : jsonData[state]["cartridge"]["prgRam"]) {
    cartridge.WritePrgRAM(entry[0], entry[1]);
  }

  // PPU
  ppu.scanline = jsonData[state]["ppu"]["scanline"];
  ppu.cycle = jsonData[state]["ppu"]["cycle"];
  ppu.ppuCtrl.value = jsonData[state]["ppu"]["ctrl"];
  ppu.ppuMask.value = jsonData[state]["ppu"]["mask"];
  ppu.ppuStatus.value = jsonData[state]["ppu"]["status"];
  ppu.oamAddr = jsonData[state]["ppu"]["oamAddr"];
  ppu.ppuScroll = jsonData[state]["ppu"]["scroll"];
  ppu.ppuAddr = jsonData[state]["ppu"]["addr"];
  ppu.ppuData = jsonData[state]["ppu"]["data"];
  ppu.vramAddr.value = jsonData[state]["ppu"]["vramAddr"];
  ppu.tempAddr.value = jsonData[state]["ppu"]["tempAddr"];
  ppu.fineX = jsonData[state]["ppu"]["fineX"];
  ppu.addrLatch = jsonData[state]["ppu"]["addrLatch"];

  //VRAM dump
  for (const auto &entry : jsonData[state]["ppu"]["vram"]) {
    u16 addr = entry[0];
    u8 val = entry[1];
    ppu.WriteVram(addr, val);
  }

  return true;
}

bool Bus::SaveStateToJson(const std::string& relativePath, const std::string& state)
{
  nlohmann::json jsonData;

  //CPU values
  jsonData[state]["cpu"] = {
    {"a", cpu.GetAccumulator()},
    {"x", cpu.GetXRegister()},
    {"y", cpu.GetYRegister()},
    {"p", cpu.GetStatusRegister()},
    {"s", cpu.GetStackPointer()},
    {"pc", cpu.GetProgramCounter()},
    {"cycles", cpu.GetCycles()}
  };

  //CPU dump
  jsonData[state]["cpu"]["ram"] = nlohmann::json::array();
  for (u16 addr = 0; addr < 0x0800; ++addr) {
    jsonData[state]["cpu"]["ram"].push_back({addr, cpu.Read(addr)});
  }

  //PPU values
  jsonData[state]["ppu"] = {
    {"ctrl", ppu.ppuCtrl.value},
    {"mask", ppu.ppuMask.value},
    {"status", ppu.ppuStatus.value},
    {"oamAddr", ppu.oamAddr},
    {"scroll", ppu.ppuScroll},
    {"addr", ppu.ppuAddr},
    {"data", ppu.ppuData},
    {"vramAddr", ppu.vramAddr.value},
    {"tempAddr", ppu.tempAddr.value},
    {"fineX", ppu.fineX},
    {"addrLatch", ppu.addrLatch},
    {"scanline", ppu.scanline},
    {"cycle", ppu.cycle}
  };

  //PPU dump
  jsonData[state]["ppu"]["vram"] = nlohmann::json::array();
  for (u16 addr = 0; addr < 0x4000; ++addr) {
    jsonData[state]["ppu"]["vram"].push_back({addr, ppu.ReadVram(addr)});
  }

  //cartridge dump
  jsonData[state]["cartridge"]["romName"] = cartridge.GetRomName();
  jsonData[state]["cartridge"]["prgRam"] = nlohmann::json::array();
  for (u16 addr = 0; addr < cartridge.GetPrgRamSize(); ++addr) {
    jsonData[state]["cartridge"]["prgRam"].push_back({addr, cartridge.ReadPrgRAM(addr)});
  }

  //save to file
  std::filesystem::path path = std::filesystem::current_path() / relativePath;
  std::ofstream file(path);
  if (!file.is_open()) return false;

  file << std::setw(2) << jsonData;
  return true;
}
