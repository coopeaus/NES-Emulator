#pragma once
#include <cstdint>
#include <cstring>
#include <string>

using u8 = uint8_t;
using u16 = uint16_t;

class iNes2Instance // NOLINT
{
  public:
    iNes2Instance() = default;

    /*
    ################################
    ||                            ||
    ||      Byte Definitions      ||
    ||                            ||
    ################################
    */

    /*
    6.  Flags 6
        D~7654 3210
          ---------
          NNNN FTBM
          |||| |||+-- Hard-wired nametable layout
          |||| |||     0: Vertical arrangement ("mirrored horizontally") or mapper-controlled
          |||| |||     1: Horizontal arrangement ("mirrored vertically")
          |||| ||+--- "Battery" and other non-volatile memory
          |||| ||      0: Not present
          |||| ||      1: Present
          |||| |+--- 512-byte Trainer
          |||| |      0: Not present
          |||| |      1: Present between Header and PRG-ROM data
          |||| +---- Alternative nametables
          ||||        0: No
          ||||        1: Yes
          ++++------ Mapper Number D3..D0
       */
    union Flag6 {
        struct {
            u8 mirroring : 1;
            u8 battery : 1;
            u8 trainer : 1;
            u8 fourScreen : 1;
            u8 mapperLSB : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    7.  Flags 7
        D~7654 3210
          ---------
          NNNN 10TT
          |||| ||++- Console type
          |||| ||     0: Nintendo Entertainment System/Family Computer
          |||| ||     1: Nintendo Vs. System
          |||| ||     2: Nintendo Playchoice 10
          |||| ||     3: Extended Console Type
          |||| ++--- NES 2.0 identifier
          ++++------ Mapper Number D7..D4
       */
    union Flag7 {
        struct {
            u8 consoleType : 2;
            u8 nes20 : 2;
            u8 mapperMid : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    8. Mapper MSB/Submapper
       D~7654 3210
         ---------
         SSSS NNNN
         |||| ++++- Mapper number D11..D8
         ++++------ Submapper number
    */
    union MapperMSB {
        struct {
            u8 submapper : 4;
            u8 mapperMSB : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    9.  PRG-ROM/CHR-ROM size MSB
        D~7654 3210
          ---------
          CCCC PPPP
          |||| ++++- PRG-ROM size MSB
          ++++------ CHR-ROM size MSB
        */
    union RomSizeMSB {
        struct {
            u8 chrRomSize : 4;
            u8 prgRomSize : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    10. PRG-RAM/EEPROM size
        D~7654 3210
          ---------
          pppp PPPP
          |||| ++++- PRG-RAM (volatile) shift count
          ++++------ PRG-NVRAM/EEPROM (non-volatile) shift count
        If the shift count is zero, there is no PRG-(NV)RAM.
        If the shift count is non-zero, the actual size is
        "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7.
    */
    union PrgRamSize {
        struct {
            u8 prgRamSize : 4;
            u8 prgNvramSize : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    11. CHR-RAM size
        D~7654 3210
          ---------
          cccc CCCC
          |||| ++++- CHR-RAM size (volatile) shift count
          ++++------ CHR-NVRAM size (non-volatile) shift count
        If the shift count is zero, there is no CHR-(NV)RAM.
        If the shift count is non-zero, the actual size is
        "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7.
    */
    union ChrRamSize {
        struct {
            u8 chrRamSize : 4;
            u8 chrNvramSize : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    12. CPU/PPU Timing
        D~7654 3210
          ---------
          .... ..VV
                ++- CPU/PPU timing mode
                      0: RP2C02 ("NTSC NES")
                      1: RP2C07 ("Licensed PAL NES")
                      2: Multiple-region
                      3: UA6538 ("Dendy")
    */
    union CpuPpuTiming {
        struct {
            u8 cpuPpuTiming : 2;
        } bit;
        u8 value = 0x00;
    };

    /*
    13. When Byte 7 AND 3 =1: Vs. System Type
        D~7654 3210
          ---------
          MMMM PPPP
          |||| ++++- Vs. PPU Type
          ++++------ Vs. Hardware Type

        When Byte 7 AND 3 =3: Extended Console Type
        D~7654 3210
          ---------
          .... CCCC
                ++++- Extended Console Type
       */
    union VsSystemType {
        struct {
            u8 vsPpuType : 4;
            u8 vsHardwareType : 4;
        } bit;
        u8 value = 0x00;
    };

    /*
    14. Miscellaneous ROMs
        D~7654 3210
          ---------
          .... ..RR
                  ++- Number of miscellaneous ROMs present
    */
    union MiscRoms {
        struct {
            u8 miscRoms : 2;
        } bit;
        u8 value = 0x00;
    };

    /*
    15. Default Expansion Device
        D~7654 3210
          ---------
          ..DD DDDD
            ++-++++- Default Expansion Device
    */
    union DefaultExpansionDevice {
        struct {
            u8 defaultExpansionDevice : 5;
        } bit;
        u8 value = 0x00;
    };

    /*
    ################################
    ||                            ||
    ||      Header Definition     ||
    ||                            ||
    ################################
    */
    // combining all the above into a 16 bit union
    union iNes2Format {
        struct {
            char                   identification[4]; // Bytes 0-3: Must be "NES\x1A"
            u8                     prgRomSizeLSB;     // Byte 4: PRG-ROM size (LSB)
            u8                     chrRomSizeLSB;     // Byte 5: CHR-ROM size (LSB)
            Flag6                  flag6;             // Byte 6: Flags 6
            Flag7                  flag7;             // Byte 7: Flags 7
            MapperMSB              mapperMSB;         // Byte 8: Mapper MSB / Submapper
            RomSizeMSB             romSizeMSB;        // Byte 9: PRG/CHR-ROM size (MSB)
            PrgRamSize             prgRamSize;        // Byte 10: PRG-RAM/EEPROM size
            ChrRamSize             chrRamSize;        // Byte 11: CHR-RAM size
            CpuPpuTiming           cpuPpuTiming;      // Byte 12: CPU/PPU Timing
            VsSystemType           vsSystemType;      // Byte 13: Vs. System Type or Extended Console Type
            MiscRoms               miscRoms;          // Byte 14: Miscellaneous ROMs
            DefaultExpansionDevice defaultExpansionDevice; // Byte 15: Default Expansion Device
        } fields;
        u8 value[16]{};
        iNes2Format() { memset( value, 0, sizeof( value ) ); }
    };

    iNes2Format header;

    /*
    ################################
    ||                            ||
    ||           Getters          ||
    ||                            ||
    ################################
    */

    // Getters in order as they appear
    std::string GetBytes0to3() const { return { header.fields.identification, 4 }; }
    int         GetByte4() const { return header.fields.prgRomSizeLSB; }
    int         GetByte5() const { return header.fields.chrRomSizeLSB; }
    int         GetByte6() const { return header.fields.flag6.value; }
    int         GetByte7() const { return header.fields.flag7.value; }
    int         GetByte8() const { return header.fields.mapperMSB.value; }
    int         GetByte9() const { return header.fields.romSizeMSB.value; }
    int         GetByte10() const { return header.fields.prgRamSize.value; }
    int         GetByte11() const { return header.fields.chrRamSize.value; }
    int         GetByte12() const { return header.fields.cpuPpuTiming.value; }
    int         GetByte13() const { return header.fields.vsSystemType.value; }
    int         GetByte14() const { return header.fields.miscRoms.value; }
    int         GetByte15() const { return header.fields.defaultExpansionDevice.value; }

    std::string GetIdentification() const { return { header.fields.identification, 4 }; }
    int         GetPrgRomBanks() const
    {
        int const msb = header.fields.romSizeMSB.bit.prgRomSize; // upper 4 bits
        int const lsb = header.fields.prgRomSizeLSB;             // full 8 bits
        return ( msb << 8 ) | lsb;
    }
    int GetPrgRomSizeBytes() const { return GetPrgRomBanks() * 16384; }
    int GetChrRomBanks() const
    {
        int const msb = header.fields.romSizeMSB.bit.chrRomSize; // upper 4 bits
        int const lsb = header.fields.chrRomSizeLSB;             // full 8 bits
        return ( msb << 8 ) | lsb;
    }
    int GetChrRomSizeBytes() const { return GetChrRomBanks() * 8192; }
    int GetMapper() const
    {
        int const msb = header.fields.mapperMSB.bit.mapperMSB;
        int const mid = header.fields.flag7.bit.mapperMid;
        int const lsb = header.fields.flag6.bit.mapperLSB;
        return ( msb << 8 ) | ( mid << 4 ) | lsb;
    }
    int GetMirroring() const { return header.fields.flag6.bit.mirroring; }
    int GetBatteryMode() const { return header.fields.flag6.bit.battery; }
    int GetTrainerMode() const { return header.fields.flag6.bit.trainer; }
    int GetFourScreenMode() const { return header.fields.flag6.bit.fourScreen; }

    // Console type and NES 2.0 identifier from Flag7.
    int GetConsoleType() const { return header.fields.flag7.bit.consoleType; }
    int GetNes20Identifier() const { return header.fields.flag7.bit.nes20; }

    // Submapper from MapperMSB.
    int GetSubmapper() const { return header.fields.mapperMSB.bit.submapper; }

    // PRG-RAM / EEPROM size.
    // The shift count is stored. If zero, there is no PRG-(NV)RAM.
    // Otherwise, the actual size in bytes is (64 << shift).
    int GetPrgRamSizeBytes() const
    {
        int const shift = header.fields.prgRamSize.bit.prgRamSize;
        return ( shift == 0 ? 0 : ( 64 << shift ) );
    }
    int GetPrgNvramSizeBytes() const
    {
        int const shift = header.fields.prgRamSize.bit.prgNvramSize;
        return ( shift == 0 ? 0 : ( 64 << shift ) );
    }

    // CHR-RAM / NVRAM size.
    int GetChrRamSizeBytes() const
    {
        int const shift = header.fields.chrRamSize.bit.chrRamSize;
        return ( shift == 0 ? 0 : ( 64 << shift ) );
    }
    int GetChrNvramSizeBytes() const
    {
        int const shift = header.fields.chrRamSize.bit.chrNvramSize;
        return ( shift == 0 ? 0 : ( 64 << shift ) );
    }

    // CPU/PPU Timing (Byte 12)
    int GetCpuPpuTiming() const { return header.fields.cpuPpuTiming.bit.cpuPpuTiming; }
    int GetRegion() const { return GetCpuPpuTiming(); }

    // Vs. System Type or Extended Console Type (Byte 13)
    int GetVsPpuType() const { return header.fields.vsSystemType.bit.vsPpuType; }
    int GetVsHardwareType() const { return header.fields.vsSystemType.bit.vsHardwareType; }

    // Miscellaneous ROMs (Byte 14)
    int GetMiscRoms() const { return header.fields.miscRoms.bit.miscRoms; }

    // Default Expansion Device (Byte 15)
    int GetDefaultExpansionDevice() const
    {
        return header.fields.defaultExpansionDevice.bit.defaultExpansionDevice;
    }

    /*
    ################################
    ||                            ||
    ||           Setters          ||
    ||                            ||
    ################################
    */
    void IncrementByte4() { header.fields.prgRomSizeLSB++; }
    void IncrementByte5() { header.fields.chrRomSizeLSB++; }
    void IncrementByte6() { header.fields.flag6.value++; }
    void IncrementByte7() { header.fields.flag7.value++; }
    void IncrementByte8() { header.fields.mapperMSB.value++; }
    void IncrementByte9() { header.fields.romSizeMSB.value++; }
    void IncrementByte10() { header.fields.prgRamSize.value++; }
    void IncrementByte11() { header.fields.chrRamSize.value++; }
    void IncrementByte12() { header.fields.cpuPpuTiming.value++; }
    void IncrementByte13() { header.fields.vsSystemType.value++; }
    void IncrementByte14() { header.fields.miscRoms.value++; }
    void IncrementByte15() { header.fields.defaultExpansionDevice.value++; }

    void DecrementByte4() { header.fields.prgRomSizeLSB--; }
    void DecrementByte5() { header.fields.chrRomSizeLSB--; }
    void DecrementByte6() { header.fields.flag6.value--; }
    void DecrementByte7() { header.fields.flag7.value--; }
    void DecrementByte8() { header.fields.mapperMSB.value--; }
    void DecrementByte9() { header.fields.romSizeMSB.value--; }
    void DecrementByte10() { header.fields.prgRamSize.value--; }
    void DecrementByte11() { header.fields.chrRamSize.value--; }
    void DecrementByte12() { header.fields.cpuPpuTiming.value--; }
    void DecrementByte13() { header.fields.vsSystemType.value--; }
    void DecrementByte14() { header.fields.miscRoms.value--; }
    void DecrementByte15() { header.fields.defaultExpansionDevice.value--; }
};
