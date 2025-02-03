#pragma once
#include <cstddef>
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

enum class MirrorMode : u8 { Horizontal, Vertical, SingleLower, SingleUpper, FourScreen };

class Mapper
{
    /**
     * @brief Mapper base class
     * All other mappers will inherit from this class
     */
  public:
    Mapper( u8 prgRomBanks, u8 chrRomBanks ) : _prgRomBanks( prgRomBanks ), _chrRomBanks( chrRomBanks ) {}

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
    [[nodiscard]] size_t GetPrgBankCount() const { return _prgRomBanks; }
    [[nodiscard]] size_t GetChrBankCount() const { return _chrRomBanks; }

    // Base methods
    virtual u32  TranslateCPUAddress( u16 address ) = 0;
    virtual u32  TranslatePPUAddress( u16 address ) = 0;
    virtual void HandleCPUWrite( u16 address, u8 data ) = 0;

    [[nodiscard]] virtual bool SupportsPrgRam() = 0;
    [[nodiscard]] virtual bool HasExpansionRom() = 0;
    [[nodiscard]] virtual bool HasExpansionRam() = 0;

    [[nodiscard]] virtual MirrorMode GetMirrorMode() = 0;

  private:
    u8 _prgRomBanks;
    u8 _chrRomBanks;
};
