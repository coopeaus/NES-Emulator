#pragma once
#include <cstddef>
#include <cstdint>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

/**
 * @brief Mapper base class
 * All other mappers will inherit from this class
 */
class Mapper
{
  public:
    Mapper( size_t prg_size_bytes, size_t chr_size_bytes )
        : _prg_KiB_capacity( prg_size_bytes ), _chr_KiB_capacity( chr_size_bytes )
    {
    }

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
    [[nodiscard]] size_t GetPrgSize() const { return _prg_KiB_capacity; }
    [[nodiscard]] size_t GetChrSize() const { return _chr_KiB_capacity; }

    // Base methods
    virtual u16                TranslateCPUAddress( u16 address ) = 0;
    virtual u16                TranslatePPUAddress( u16 address ) = 0;
    virtual void               HandleCPUWrite( u16 address, u8 data ) = 0;
    [[nodiscard]] virtual u8   GetMirrorMode() = 0;
    [[nodiscard]] virtual bool HasPrgRam() = 0;
    [[nodiscard]] virtual bool HasExpansionRom() = 0;
    [[nodiscard]] virtual bool HasExpansionRam() = 0;

  private:
    size_t _prg_KiB_capacity;
    size_t _chr_KiB_capacity;
};
