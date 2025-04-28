#pragma once
#include "global-types.h"
#include <string>
#include <bitset>

// clang-format off
#include <array>
const std::array<std::string, 256> gInstructionNames = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
    /*0*/"BRK",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "PHP",  "ORA",   "ASL",   "*ANC",  "*NOP",  "ORA",   "ASL",   "*SLO",
    /*1*/"BPL",   "ORA",   "*JAM",  "*SLO",  "*NOP",  "ORA",   "*ASL",  "*SLO",  "CLC",  "ORA",   "*NOP",  "*SLO",  "*NOP",  "ORA",   "ASL",   "*SLO",
    /*2*/"JSR",   "AND",   "*JAM",  "*RLA",  "BIT",   "AND",   "ROL",   "*RLA",  "PLP",  "AND",   "ROL",   "*ANC",  "BIT",   "AND",   "ROL",   "*RLA",
    /*3*/"BMI",   "AND",   "*JAM",  "*RLA",  "*NOP",  "AND",   "*ROL",  "*RLA",  "SEC",  "AND",   "*NOP",  "*RLA",  "*NOP",  "AND",   "ROL",   "*RLA",
    /*4*/"RTI",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "PHA",  "EOR",   "LSR",   "*ALR",  "JMP",   "EOR",   "LSR",   "*SRE",
    /*5*/"BVC",   "EOR",   "*JAM",  "*SRE",  "*NOP",  "EOR",   "*LSR",  "*SRE",  "CLI",  "EOR",   "*NOP",  "*SRE",  "*NOP",  "EOR",   "LSR",   "*SRE",
    /*6*/"RTS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "PLA",  "ADC",   "ROR",   "*ARR",  "JMP",   "ADC",   "ROR",   "*RRA",
    /*7*/"BVS",   "ADC",   "*JAM",  "*RRA",  "*NOP",  "ADC",   "*ROR",  "*RRA",  "SEI",  "ADC",   "*NOP",  "*RRA",  "*NOP",  "ADC",   "ROR",   "*RRA",
    /*8*/"*NOP",  "STA",   "*NOP",  "*SAX",  "STY",   "STA",   "STX",   "*SAX",  "DEY",  "*NOP",  "TXA",   "*ANE",  "STY",   "STA",   "STX",   "*SAX",
    /*9*/"BCC",   "STA",   "*JAM",  "*SHA",  "STY",   "STA",   "STX",   "*SAX",  "TYA",  "STA",   "TXS",   "*TAS",  "*SHY",  "STA",   "*SHX",  "*SHA",
    /*A*/"LDY",   "LDA",   "LDX",   "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "TAY",  "LDA",   "TAX",   "*LXA",  "LDY",   "LDA",   "LDX",   "*LAX",
    /*B*/"BCS",   "LDA",   "*JAM",  "*LAX",  "LDY",   "LDA",   "LDX",   "*LAX",  "CLV",  "LDA",   "TSX",   "*LAS",  "LDY",   "LDA",   "LDX",   "*LAX",
    /*C*/"CPY",   "CMP",   "*NOP",  "*DCP",  "CPY",   "CMP",   "DEC",   "*DCP",  "INY",  "CMP",   "DEX",   "*SBX",  "CPY",   "CMP",   "DEC",   "*DCP",
    /*D*/"BNE",   "CMP",   "*JAM",  "*DCP",  "*NOP",  "CMP",   "*DEC",  "*DCP",  "CLD",  "CMP",   "*NOP",  "*DCP",  "*NOP",  "CMP",   "DEC",   "*DCP",
    /*E*/"CPX",   "SBC",   "*NOP",  "*ISC",  "CPX",   "SBC",   "INC",   "*ISC",  "INX",  "SBC",   "NOP",   "*SBC",  "CPX",   "SBC",   "INC",   "*ISC",
    /*F*/"BEQ",   "SBC",   "*JAM",  "*ISC",  "*NOP",  "SBC",   "*INC",  "*ISC",  "SED",  "SBC",   "*NOP",  "*ISC",  "*NOP",  "SBC",   "INC",   "*ISC"
};

const std::array<std::string, 256> gAddressingModes = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*1*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*2*/"ABS",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*3*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*4*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*5*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*6*/"IMP",   "INDX",  "IMP",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "IND",   "ABS",   "ABS",   "ABS",
     /*7*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*8*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*9*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
     /*A*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*B*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGY",  "ZPGY",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSY",  "ABSY",
     /*C*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*D*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX",
     /*E*/"IMM",   "INDX",  "IMM",   "INDX",  "ZPG",   "ZPG",   "ZPG",   "ZPG",   "IMP",  "IMM",   "IMP",   "IMM",   "ABS",   "ABS",   "ABS",   "ABS",
     /*F*/"REL",   "INDY",  "IMP",   "INDY",  "ZPGX",  "ZPGX",  "ZPGX",  "ZPGX",  "IMP",  "ABSY",  "IMP",   "ABSY",  "ABSX",  "ABSX",  "ABSX",  "ABSX"
};

const std::array<u8, 256> gInstructionCycles = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/7,       6,       2,       8,       3,       3,       5,       5,       3,       2,       2,      2,       4,       4,       6,       6,
     /*1*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*2*/6,       6,       2,       8,       3,       3,       5,       5,       4,       2,       2,      2,       4,       4,       6,       6,
     /*3*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*4*/6,       6,       2,       8,       3,       3,       5,       5,       3,       2,       2,      2,       3,       4,       6,       6,
     /*5*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*6*/6,       6,       2,       8,       3,       3,       5,       5,       4,       2,       2,      2,       5,       4,       6,       6,
     /*7*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*8*/2,       6,       2,       6,       3,       3,       3,       3,       2,       2,       2,      2,       4,       4,       4,       4,
     /*9*/2,       6,       2,       6,       4,       4,       4,       4,       2,       5,       2,      5,       5,       5,       5,       5,
     /*A*/2,       6,       2,       6,       3,       3,       3,       3,       2,       2,       2,      2,       4,       4,       4,       4,
     /*B*/2,       5,       2,       5,       4,       4,       4,       4,       2,       4,       2,      4,       4,       4,       4,       4,
     /*C*/2,       6,       2,       8,       3,       3,       5,       5,       2,       2,       2,      2,       4,       4,       6,       6,
     /*D*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7,
     /*E*/2,       6,       2,       8,       3,       3,       5,       5,       2,       2,       2,      2,       4,       4,       6,       6,
     /*F*/2,       5,       2,       8,       4,       4,       6,       6,       2,       4,       2,      7,       4,       4,       7,       7
};

const std::array<u8, 256> gInstructionBytes = {
          //0      1        2        3        4        5        6        7        8        9        A       B        C        D        E        F
     /*0*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*1*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*2*/3,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*3*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*4*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*5*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*6*/1,       2,       1,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*7*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*8*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*9*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*A*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*B*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*C*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*D*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3,
     /*E*/2,       2,       2,       2,       2,       2,       2,       2,       1,       2,       1,      2,       3,       3,       3,       3,
     /*F*/2,       2,       1,       2,       2,       2,       2,       2,       1,       3,       1,      3,       3,       3,       3,       3
};

inline const std::bitset<256> noPageCrossPenaltyOpcodes = []() {
    std::bitset<256> bs;
    bs.set( 0x9D ); bs.set( 0x99 ); bs.set( 0x81 ); bs.set( 0x91 ); bs.set( 0xFE ); bs.set( 0xDE );
    bs.set( 0x1E ); bs.set( 0x5E ); bs.set( 0x3E ); bs.set( 0x7E ); bs.set( 0x1F ); bs.set( 0x1B );
    bs.set( 0x13 ); bs.set( 0x3F ); bs.set( 0x3B ); bs.set( 0x33 ); bs.set( 0x5F ); bs.set( 0x5B );
    bs.set( 0x53 ); bs.set( 0x7F ); bs.set( 0x7B ); bs.set( 0x73 ); bs.set( 0xDF ); bs.set( 0xDB );
    bs.set( 0xD3 ); bs.set( 0xFF ); bs.set( 0xFB ); bs.set( 0xF3 );
    return bs;
}();

inline const std::bitset<256> writeModifyOpcodes = []() {
    std::bitset<256> bs;
    bs.set( 0xB6 ); bs.set( 0x9D ); bs.set( 0x99 ); bs.set( 0x91 ); bs.set( 0x96 ); bs.set( 0xFE );
    bs.set( 0xDE ); bs.set( 0x1E ); bs.set( 0x5E ); bs.set( 0x3E ); bs.set( 0x7E ); bs.set( 0x1F );
    bs.set( 0x1B ); bs.set( 0x13 ); bs.set( 0x3F ); bs.set( 0x3B ); bs.set( 0x33 ); bs.set( 0x5F );
    bs.set( 0x5B ); bs.set( 0x53 ); bs.set( 0x7F ); bs.set( 0x7B ); bs.set( 0x73 ); bs.set( 0x97 );
    bs.set( 0xB7 ); bs.set( 0xDF ); bs.set( 0xDB ); bs.set( 0xD3 ); bs.set( 0xFF ); bs.set( 0xFB );
    bs.set( 0xF3 );
    return bs;
}();
// clang-format on

inline bool isPageCrossPenalty( u8 opcode )
{
  return !noPageCrossPenaltyOpcodes.test( opcode );
}

inline bool isWriteModify( u8 opcode )
{
  return writeModifyOpcodes.test( opcode );
}
