#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

class Mapper4 : public Mapper {
public:
  Mapper4(uint8_t prgBanks, uint8_t chrBanks);
  Mapper4( iNes2Instance iNesHeader );
  uint16_t MapReadPRG(uint16_t addr) ;
  bool MapWritePRG(uint16_t addr, uint8_t data) ;

  uint16_t MapReadCHR(uint16_t address) ;
  bool MapWriteCHR(uint16_t addr, uint8_t data) ;

  void TickScanlineCounter(uint16_t ppuAddr) override;
  void ClockIRQ() override;
  void Reset();

  bool HasScanlineIRQ() const override { return true; }

  /*
################################
||        Mapper Methods      ||
################################
*/
  u32  TranslateCPUAddress( u16 address ) override;
  u32  TranslatePPUAddress( u16 address ) override;
  void HandleCPUWrite( u16 address, u8 data ) override;

  [[nodiscard]] bool       SupportsPrgRam() override { return false; }
  [[nodiscard]] bool HasExpansionRom() override;
  [[nodiscard]] MirrorMode GetMirrorMode() override;
  [[nodiscard]] bool       HasExpansionRam() override { return false; }



private:

  /*
################################
||        Bank & IRQ logic    ||
################################
*/
  void UpdateBanks();
  void UpdateIRQ();

  uint8_t bankSelect = 0;
  uint8_t bankRegisters[8] = {};
  bool prgMode = false;
  bool chrMode = false;
  bool IsIRQPending() const;

  bool chrInversion = false;

  uint8_t irqLatch = 0;
  uint8_t irqCounter = 0;
  bool irqReload = false;
  bool irqEnable = false;
  bool prevA12 = false;

  u8 prgBanks[4] = {};  //8KB PRG banks
  u8 chrBanks[8] = {};  //1KB CHR banks

  // Mirroring
  MirrorMode mirror = MirrorMode::Horizontal;
};