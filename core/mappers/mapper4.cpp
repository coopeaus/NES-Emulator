#include "mapper4.h"
#include "bus.h"
#include "cartridge-header.h"
#include "mappers/mapper-base.h"
#include "global-types.h"
#include <algorithm>

Mapper4::Mapper4( iNes2Instance iNesHeader ) : Mapper( iNesHeader )
{
  Reset();

}

void Mapper4::Reset()
{
  //Reset mapper
  bankSelect = 0;
  prgMode = false;
  chrInversion = false;
  irqEnable = false;
  irqCounter = 0;
  irqLatch = 0;
  irqReload = false;
  prevA12 = false;

  // Set default
std::fill(std::begin(bankRegisters), std::end(bankRegisters), 0);
  bankRegisters[7] = 1;
  bankRegisters[6] = 0;
  //run
  UpdateBanks();
}

//temporary todo
bool Mapper4::HasExpansionRom() {
  return false;
}
bool Mapper4::MapWriteCHR(uint16_t address, uint8_t data) {
  return false;
}


uint16_t Mapper4::MapReadPRG(uint16_t addr) {
    if (addr >= 0x8000 && addr <= 0x9FFF)
        return (prgMode ? (GetPrgBankCount() - 2) : bankRegisters[6]) * 0x2000 + (addr & 0x1FFF);
    else if (addr >= 0xA000 && addr <= 0xBFFF)
        return bankRegisters[7] * 0x2000 + (addr & 0x1FFF);
    else if (addr >= 0xC000 && addr <= 0xDFFF)
        return (prgMode ? bankRegisters[6] : (GetPrgBankCount() - 2)) * 0x2000 + (addr & 0x1FFF);
    else if (addr >= 0xE000)
        return (GetPrgBankCount() - 1) * 0x2000 + (addr & 0x1FFF); // final mem blocks
    return 0;
}

bool Mapper4::MapWritePRG(uint16_t address, uint8_t data) {
    switch (address & 0xE001) {
        case 0x8000:
            bankSelect = data & 0x07;
            prgMode = data & 0x40;
            chrInversion = data & 0x80;
            break;
        case 0x8001:
            bankRegisters[bankSelect] = data;
            UpdateBanks();
            break;
        case 0xA000:
            if (bus) bus->cartridge.SetMirrorMode((data & 1) ? MirrorMode::Horizontal : MirrorMode::Vertical);
            break;
        case 0xC000:
            irqLatch = data;
            break;
        case 0xC001:
            irqReload = true;
            break;
        case 0xE000:
            irqEnable = false;
            if (bus) bus->cpu.ClearIRQ(); //reset
            break;
        case 0xE001:
            irqEnable = true;
            break;
    }
    return true;
}

uint16_t Mapper4::MapReadCHR(uint16_t address) {
    uint8_t bank = 0;
    if (chrInversion) {
        if (address < 0x0400) bank = bankRegisters[2];
        else if (address < 0x0800) bank = bankRegisters[3];
        else if (address < 0x0C00) bank = bankRegisters[4];
        else bank = bankRegisters[5];
    } else {
        if (address < 0x0800) bank = bankRegisters[0] & 0xFE;
        else bank = bankRegisters[1] & 0xFE;
    }
    return bank * 0x400 + (address & 0x03FF);
}



void Mapper4::TickScanlineCounter(uint16_t ppuAddr) {
    bool currentA12 = (ppuAddr & 0x1000) != 0;
    if (!prevA12 && currentA12) {
      ClockIRQ();
        if (irqCounter == 0 || irqReload) {
            irqCounter = irqLatch;
            irqReload = false;
        } else {
            --irqCounter;
        }

      if (irqCounter == 0 && irqEnable && bus && !bus->cpu.IsIRQPending()) {
        bus->cpu.TriggerIRQ();
      }
    }
    prevA12 = currentA12;
}

void Mapper4::ClockIRQ() {
    if (irqCounter == 0 || irqReload) {
        irqCounter = irqLatch;
        irqReload = false;
    } else {
        --irqCounter;
    }

    if (irqCounter == 0 && irqEnable && bus) {
        bus->cpu.TriggerIRQ();
    }
}
bool Mapper4::IsIRQPending() const {
  return irqCounter == 0 && irqEnable;
}
void Mapper4::UpdateBanks() {
  const int prgCount = GetPrgBankCount();
    const int chrCount = GetChrBankCount();

    // limiter
    auto PrgLimit = [&](u8 val) { return prgCount ? val % prgCount : 0; };
    auto ChrLimit = [&](u8 val) { return chrCount ? val % chrCount : 0; };

    //PRG ROM Mapping
    if (prgMode) {
      //Fixed
        prgBanks[0] = PrgLimit(prgCount - 2);
      //Switch
        prgBanks[1] = PrgLimit(bankRegisters[6]);
    } else {
      //Fixed
        prgBanks[0] = PrgLimit(bankRegisters[6]);
      //Switch
        prgBanks[1] = PrgLimit(prgCount - 2);
    }
    prgBanks[2] = PrgLimit(bankRegisters[7]);
  prgBanks[3] = (prgCount > 0) ? prgCount - 1 : 0;

    //CHR ROM Mapping
    if (chrInversion) {
        chrBanks[0] = ChrLimit(bankRegisters[2]);
        chrBanks[1] = ChrLimit(bankRegisters[3]);
        chrBanks[2] = ChrLimit(bankRegisters[4]);
        chrBanks[3] = ChrLimit(bankRegisters[5]);
        chrBanks[4] = ChrLimit(bankRegisters[0] & 0xFE);
        chrBanks[5] = ChrLimit((bankRegisters[0] & 0xFE) + 1);
        chrBanks[6] = ChrLimit(bankRegisters[1] & 0xFE);
        chrBanks[7] = ChrLimit((bankRegisters[1] & 0xFE) + 1);
    } else {
        chrBanks[0] = ChrLimit(bankRegisters[0] & 0xFE);
        chrBanks[1] = ChrLimit((bankRegisters[0] & 0xFE) + 1);
        chrBanks[2] = ChrLimit(bankRegisters[1] & 0xFE);
        chrBanks[3] = ChrLimit((bankRegisters[1] & 0xFE) + 1);
        chrBanks[4] = ChrLimit(bankRegisters[2]);
        chrBanks[5] = ChrLimit(bankRegisters[3]);
        chrBanks[6] = ChrLimit(bankRegisters[4]);
        chrBanks[7] = ChrLimit(bankRegisters[5]);
    }

}
u32 Mapper4::TranslateCPUAddress(u16 address) {
  u32 bank = 0;
  u32 offset = address & 0x1FFF;

  if (address >= 0x8000 && address <= 0x9FFF)
    bank = prgBanks[0];
  else if (address >= 0xA000 && address <= 0xBFFF)
    bank = prgBanks[1];
  else if (address >= 0xC000 && address <= 0xDFFF)
    bank = prgBanks[2];
  else if (address >= 0xE000)
    bank = prgBanks[3];

  u32 finalAddress = (bank * 0x2000) + offset;


  return finalAddress;

}

u32 Mapper4::TranslatePPUAddress(u16 address) {
    if (address <= 0x1FFF) {
        u8 index = 0;
        if (chrInversion) {
            if (address < 0x0800) {
                index = 2 + ((address >> 10) & 0x03);
            } else {
                index = (address < 0x1000) ? 0 : 1;
            }
        } else {
            if (address < 0x0800) {
                index = (address < 0x0400) ? 0 : 1;
            } else {
                index = 2 + ((address >> 10) & 0x03);
            }
        }//final address
        return chrBanks[index] * 0x400 + (address & 0x3FF);
    }
    return 0;
}

void Mapper4::HandleCPUWrite(u16 address, u8 data) {
    switch (address & 0xE001) {
        case 0x8000:
            bankSelect = data & 0x07;
            prgMode = data & 0x40;
            chrInversion = data & 0x80;
            break;
        case 0x8001:
            bankRegisters[bankSelect] = data;
            UpdateBanks();
            break;
        case 0xA000:
            mirror = (data & 1) ? MirrorMode::Horizontal : MirrorMode::Vertical;
            break;
        case 0xC000:
            irqLatch = data;
            break;
        case 0xC001:
            irqReload = true;
            break;
        case 0xE000:
            irqEnable = false;
            if (bus) bus->cpu.ClearIRQ();
            break;
        case 0xE001:
            irqEnable = true;
            break;
    }
}

MirrorMode Mapper4::GetMirrorMode() { return mirror; }

