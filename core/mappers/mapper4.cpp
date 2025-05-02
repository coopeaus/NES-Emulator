#include "mapper4.h"
#include "cartridge-header.h"
#include "mappers/mapper-base.h"
#include "global-types.h"


//Mapper4::Mapper4( uint8_t prgBanks, uint8_t chrBanks ) : Mapper()
//{
  // TODO:init default PRG and CHR banks
//}

uint16_t Mapper4::MapReadPRG(uint16_t addr)
{
  // TODO:PRG ROM read mapping based on bank mode

  return 0;
}

uint16_t Mapper4::MapReadCHR(uint16_t addr)
{
  // TODO: Implement CHR ROM read mapping based on bank mode
  return 0;
}
//todo write chr
bool Mapper4::MapWritePRG(uint16_t addr, uint8_t data)
{
return false;

}


void Mapper4::PRGBankUpdate()
{
  // TODO: Update PRG bank mapping based on prgMode
}

void Mapper4::CHRBankUpdate()
{
  // TODO: Update CHR bank mapping based on chrInversion
}

//todo make IRQ Clock Method with CPU interupt request

