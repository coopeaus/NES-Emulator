#pragma once
#include "cartridge-header.h"
#include "global-types.h"
#include "mapper-base.h"

/**
 * Mapper 4 NesDev(MMC) implementation
 */
class Mapper4 : public Mapper
{
public:
  Mapper4(uint8_t prgBanks, uint8_t chrBanks);

  //TODO: Memory mapping
  uint16_t MapReadPRG(uint16_t addr);// override;
  uint16_t MapReadCHR(uint16_t addr);// override;

  bool MapWritePRG(uint16_t addr, uint8_t data);// override;
  bool MapWriteCHR(uint16_t addr, uint8_t data);// override;

  //TODO: Clocking and Interupt

  //TODO:save/load

private:
  //TODO:bank registers
  uint8_t prgBankSelect[4]{};
  uint8_t chrBankSelect[6]{};
  uint8_t bankMode = 0;
  uint8_t prgMode = 0;
  uint8_t chrInversion = 0;

  //TODO: Add Interupt request
  uint8_t irqLatch = 0;
  uint8_t irqCounter = 0;

  //TODO:internal state for bank switching logic
  uint8_t bankRegister = 0;
  uint8_t lastWrite = 0;

  //TODO: methods for bank switching
  void PRGBankUpdate();
  void CHRBankUpdate();
};