#pragma once
#include "cartridge-header.h"
#include "global-types.h"

enum class MirrorMode : u8 { Horizontal, Vertical, SingleLower, SingleUpper, FourScreen };

class Mapper
{
  /**
   * @brief Mapper base class
   * All other mappers will inherit from this class
   */
public:
  // header will be passed and copied. This instance is not the same as the one one in the cartridge header.
  // Each mapper will have its own copy of the iNes header. Since these don't change after rom loading, this
  // is fine. It also allows the debug window to modify the header in the cartridge header, just to see what
  // each bit does, without affecting emulation.
  iNes2Instance iNes;
  Mapper( iNes2Instance iNesHeader ) : iNes( iNesHeader ) {}

  // Delete the copy constructor
  // Prevents creating a new Mapper by copying an existing one
  Mapper( const Mapper & ) = delete;

  // Delete the copy assignment operator
  // Prevents assigning one Mapper to another by copy
  Mapper &operator=( const Mapper & ) = delete;

  // Delete the move constructor
  // Prevents creating a new Mapper by moving an existing one (std::move)
  Mapper( Mapper && ) = delete;

  // Delete the move assignment operator
  // Prevents assigning one Mapper to another by moving (std::move)
  Mapper &operator=( Mapper && ) = default;

  // Virtual destructor and polymorphic cleanup
  // Calls the derived class destructor when deleting a Mapper pointer
  virtual ~Mapper() = default;

  // Getters
  int GetPrgBankCount() const { return iNes.GetPrgRomBanks(); }
  int GetChrBankCount() const { return iNes.GetChrRomBanks(); }

  // Base methods
  virtual u32  TranslateCPUAddress( u16 address ) = 0;
  virtual u32  TranslatePPUAddress( u16 address ) = 0;
  virtual void HandleCPUWrite( u16 address, u8 data ) = 0;

  [[nodiscard]] virtual bool SupportsPrgRam() = 0;
  [[nodiscard]] virtual bool HasExpansionRom() = 0;
  [[nodiscard]] virtual bool HasExpansionRam() = 0;

  [[nodiscard]] virtual MirrorMode GetMirrorMode() = 0;

private:
};
