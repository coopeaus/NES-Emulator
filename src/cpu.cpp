// cpu.cpp

#include "bus.h"
#include "cpu.h"
#include "utils.h"
#include <cstdint>
#include <string>
#include <iostream>
#include <stdexcept>

CPU::CPU( Bus *bus ) : _bus( bus ), _opcodeTable{}
{

    /*
    ################################################################
    ||                                                            ||
    ||                           Opcodes                          ||
    ||                                                            ||
    ################################################################
    */
    // NOLINTBEGIN
    // clang-format off
    // NOP
    _opcodeTable[0xEA] = InstructionData{ "NOP", "IMM",  &CPU::NOP, &CPU::IMP,  2, 1 };

    // LDA
    _opcodeTable[0xA9] = InstructionData{ "LDA", "IMM",  &CPU::LDA, &CPU::IMM,  2, 2 };
    _opcodeTable[0xA5] = InstructionData{ "LDA", "ZPG",  &CPU::LDA, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xB5] = InstructionData{ "LDA", "ZPGX", &CPU::LDA, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xAD] = InstructionData{ "LDA", "ABS",  &CPU::LDA, &CPU::ABS,  4, 3 };
    _opcodeTable[0xBD] = InstructionData{ "LDA", "ABSX", &CPU::LDA, &CPU::ABSX, 4, 3 };
    _opcodeTable[0xB9] = InstructionData{ "LDA", "ABSY", &CPU::LDA, &CPU::ABSY, 4, 3 };
    _opcodeTable[0xA1] = InstructionData{ "LDA", "INDX", &CPU::LDA, &CPU::INDX, 6, 2 };
    _opcodeTable[0xB1] = InstructionData{ "LDA", "INDY", &CPU::LDA, &CPU::INDY, 5, 2 };

    // LDX
    _opcodeTable[0xA2] = InstructionData{ "LDX", "IMM",  &CPU::LDX, &CPU::IMM,  2, 2 };
    _opcodeTable[0xA6] = InstructionData{ "LDX", "ZPG",  &CPU::LDX, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xB6] = InstructionData{ "LDX", "ZPGY", &CPU::LDX, &CPU::ZPGY, 4, 2, true, true };
    _opcodeTable[0xAE] = InstructionData{ "LDX", "ABS",  &CPU::LDX, &CPU::ABS,  4, 3 };
    _opcodeTable[0xBE] = InstructionData{ "LDX", "ABSY", &CPU::LDX, &CPU::ABSY, 4, 3 };

    // LDY
    _opcodeTable[0xA0] = InstructionData{ "LDY", "IMM",  &CPU::LDY, &CPU::IMM,  2, 2 };
    _opcodeTable[0xA4] = InstructionData{ "LDY", "ZPG",  &CPU::LDY, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xB4] = InstructionData{ "LDY", "ZPGX", &CPU::LDY, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xAC] = InstructionData{ "LDY", "ABS",  &CPU::LDY, &CPU::ABS,  4, 3 };
    _opcodeTable[0xBC] = InstructionData{ "LDY", "ABSX", &CPU::LDY, &CPU::ABSX, 4, 3 };

    // STA
    _opcodeTable[0x85] = InstructionData{ "STA", "ZPG",  &CPU::STA, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x95] = InstructionData{ "STA", "ZPGX", &CPU::STA, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x8D] = InstructionData{ "STA", "ABS",  &CPU::STA, &CPU::ABS,  4, 3 };
    _opcodeTable[0x9D] = InstructionData{ "STA", "ABSX", &CPU::STA, &CPU::ABSX, 5, 3, false, true };
    _opcodeTable[0x99] = InstructionData{ "STA", "ABSY", &CPU::STA, &CPU::ABSY, 5, 3, false, true };
    _opcodeTable[0x81] = InstructionData{ "STA", "INDX", &CPU::STA, &CPU::INDX, 6, 2, false };
    _opcodeTable[0x91] = InstructionData{ "STA", "INDY", &CPU::STA, &CPU::INDY, 6, 2, false, true };

    // STX
    _opcodeTable[0x86] = InstructionData{ "STX", "ZPG",  &CPU::STX, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x96] = InstructionData{ "STX", "ZPGY", &CPU::STX, &CPU::ZPGY, 4, 2, true, true };
    _opcodeTable[0x8E] = InstructionData{ "STX", "ABS",  &CPU::STX, &CPU::ABS,  4, 3 };

    // STY
    _opcodeTable[0x84] = InstructionData{ "STY", "ZPG",  &CPU::STY, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x94] = InstructionData{ "STY", "ZPGX", &CPU::STY, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x8C] = InstructionData{ "STY", "ABS",  &CPU::STY, &CPU::ABS,  4, 3 };

    // ADC
    _opcodeTable[0x69] = InstructionData{ "ADC", "IMM",  &CPU::ADC, &CPU::IMM,  2, 2 };
    _opcodeTable[0x65] = InstructionData{ "ADC", "ZPG",  &CPU::ADC, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x75] = InstructionData{ "ADC", "ZPGX", &CPU::ADC, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x6D] = InstructionData{ "ADC", "ABS",  &CPU::ADC, &CPU::ABS,  4, 3 };
    _opcodeTable[0x7D] = InstructionData{ "ADC", "ABSX", &CPU::ADC, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x79] = InstructionData{ "ADC", "ABSY", &CPU::ADC, &CPU::ABSY, 4, 3 };
    _opcodeTable[0x61] = InstructionData{ "ADC", "INDX", &CPU::ADC, &CPU::INDX, 6, 2 };
    _opcodeTable[0x71] = InstructionData{ "ADC", "INDY", &CPU::ADC, &CPU::INDY, 5, 2 };

    // SBC
    _opcodeTable[0xE9] = InstructionData{ "SBC", "IMM",  &CPU::SBC, &CPU::IMM,  2, 2 };
    _opcodeTable[0xE5] = InstructionData{ "SBC", "ZPG",  &CPU::SBC, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xF5] = InstructionData{ "SBC", "ZPGX", &CPU::SBC, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xED] = InstructionData{ "SBC", "ABS",  &CPU::SBC, &CPU::ABS,  4, 3 };
    _opcodeTable[0xFD] = InstructionData{ "SBC", "ABSX", &CPU::SBC, &CPU::ABSX, 4, 3 };
    _opcodeTable[0xF9] = InstructionData{ "SBC", "ABSY", &CPU::SBC, &CPU::ABSY, 4, 3 };
    _opcodeTable[0xE1] = InstructionData{ "SBC", "INDX", &CPU::SBC, &CPU::INDX, 6, 2 };
    _opcodeTable[0xF1] = InstructionData{ "SBC", "INDY", &CPU::SBC, &CPU::INDY, 5, 2 };

    // INC
    _opcodeTable[0xE6] = InstructionData{ "INC", "ZPG",  &CPU::INC, &CPU::ZPG,  5, 2 };
    _opcodeTable[0xF6] = InstructionData{ "INC", "ZPGX", &CPU::INC, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0xEE] = InstructionData{ "INC", "ABS",  &CPU::INC, &CPU::ABS,  6, 3 };
    _opcodeTable[0xFE] = InstructionData{ "INC", "ABSX", &CPU::INC, &CPU::ABSX, 7, 3, false, true };

    // DEC
    _opcodeTable[0xC6] = InstructionData{ "DEC", "ZPG",  &CPU::DEC, &CPU::ZPG,  5, 2 };
    _opcodeTable[0xD6] = InstructionData{ "DEC", "ZPGX", &CPU::DEC, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0xCE] = InstructionData{ "DEC", "ABS",  &CPU::DEC, &CPU::ABS,  6, 3 };
    _opcodeTable[0xDE] = InstructionData{ "DEC", "ABSX", &CPU::DEC, &CPU::ABSX, 7, 3, false, true };

    // INX, INY, DEX, DEY
    _opcodeTable[0xE8] = InstructionData{ "INX", "IMP",  &CPU::INX, &CPU::IMP,  2, 1 };
    _opcodeTable[0xC8] = InstructionData{ "INY", "IMP",  &CPU::INY, &CPU::IMP,  2, 1 };
    _opcodeTable[0xCA] = InstructionData{ "DEX", "IMP",  &CPU::DEX, &CPU::IMP,  2, 1 };
    _opcodeTable[0x88] = InstructionData{ "DEY", "IMP",  &CPU::DEY, &CPU::IMP,  2, 1 };

    // CLC
    _opcodeTable[0x18] = InstructionData{ "CLC", "IMP",  &CPU::CLC, &CPU::IMP,  2, 1 };
    _opcodeTable[0x58] = InstructionData{ "CLI", "IMP",  &CPU::CLI, &CPU::IMP,  2, 1 };
    _opcodeTable[0xD8] = InstructionData{ "CLD", "IMP",  &CPU::CLD, &CPU::IMP,  2, 1 };
    _opcodeTable[0xB8] = InstructionData{ "CLV", "IMP",  &CPU::CLV, &CPU::IMP,  2, 1 };

    _opcodeTable[0x38] = InstructionData{ "SEC", "IMP",  &CPU::SEC, &CPU::IMP,  2, 1 };
    _opcodeTable[0x78] = InstructionData{ "SEI", "IMP",  &CPU::SEI, &CPU::IMP,  2, 1 };
    _opcodeTable[0xF8] = InstructionData{ "SED", "IMP",  &CPU::SED, &CPU::IMP,  2, 1 };

    // Branch
    _opcodeTable[0x10] = InstructionData{ "BPL", "REL",  &CPU::BPL, &CPU::REL,  2, 2 };
    _opcodeTable[0x30] = InstructionData{ "BMI", "REL",  &CPU::BMI, &CPU::REL,  2, 2 };
    _opcodeTable[0x50] = InstructionData{ "BVC", "REL",  &CPU::BVC, &CPU::REL,  2, 2 };
    _opcodeTable[0x70] = InstructionData{ "BVS", "REL",  &CPU::BVS, &CPU::REL,  2, 2 };
    _opcodeTable[0x90] = InstructionData{ "BCC", "REL",  &CPU::BCC, &CPU::REL,  2, 2 };
    _opcodeTable[0xB0] = InstructionData{ "BCS", "REL",  &CPU::BCS, &CPU::REL,  2, 2 };
    _opcodeTable[0xD0] = InstructionData{ "BNE", "REL",  &CPU::BNE, &CPU::REL,  2, 2 };
    _opcodeTable[0xF0] = InstructionData{ "BEQ", "REL",  &CPU::BEQ, &CPU::REL,  2, 2 };

    // CMP, CPX, CPY
    _opcodeTable[0xC9] = InstructionData{ "CMP", "IMM",  &CPU::CMP, &CPU::IMM,  2, 2 };
    _opcodeTable[0xC5] = InstructionData{ "CMP", "ZPG",  &CPU::CMP, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xD5] = InstructionData{ "CMP", "ZPGX", &CPU::CMP, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xCD] = InstructionData{ "CMP", "ABS",  &CPU::CMP, &CPU::ABS,  4, 3 };
    _opcodeTable[0xDD] = InstructionData{ "CMP", "ABSX", &CPU::CMP, &CPU::ABSX, 4, 3 };
    _opcodeTable[0xD9] = InstructionData{ "CMP", "ABSY", &CPU::CMP, &CPU::ABSY, 4, 3 };
    _opcodeTable[0xC1] = InstructionData{ "CMP", "INDX", &CPU::CMP, &CPU::INDX, 6, 2 };
    _opcodeTable[0xD1] = InstructionData{ "CMP", "INDY", &CPU::CMP, &CPU::INDY, 5, 2 };
    _opcodeTable[0xE0] = InstructionData{ "CPX", "IMM",  &CPU::CPX, &CPU::IMM,  2, 2 };
    _opcodeTable[0xE4] = InstructionData{ "CPX", "ZPG",  &CPU::CPX, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xEC] = InstructionData{ "CPX", "ABS",  &CPU::CPX, &CPU::ABS,  4, 3 };
    _opcodeTable[0xC0] = InstructionData{ "CPY", "IMM",  &CPU::CPY, &CPU::IMM,  2, 2 };
    _opcodeTable[0xC4] = InstructionData{ "CPY", "ZPG",  &CPU::CPY, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xCC] = InstructionData{ "CPY", "ABS",  &CPU::CPY, &CPU::ABS,  4, 3 };

    // PHA, PHP, PLA, PLP, TSX, TXS
    _opcodeTable[0x48] = InstructionData{ "PHA", "IMP",  &CPU::PHA, &CPU::IMP,  3, 1 };
    _opcodeTable[0x08] = InstructionData{ "PHP", "IMP",  &CPU::PHP, &CPU::IMP,  3, 1 };
    _opcodeTable[0x68] = InstructionData{ "PLA", "IMP",  &CPU::PLA, &CPU::IMP,  4, 1 };
    _opcodeTable[0x28] = InstructionData{ "PLP", "IMP",  &CPU::PLP, &CPU::IMP,  4, 1 };
    _opcodeTable[0xBA] = InstructionData{ "TSX", "IMP",  &CPU::TSX, &CPU::IMP,  2, 1 };
    _opcodeTable[0x9A] = InstructionData{ "TXS", "IMP",  &CPU::TXS, &CPU::IMP,  2, 1 };

    // ASL, LSR
    _opcodeTable[0x0A] = InstructionData{ "ASL", "IMP",  &CPU::ASL, &CPU::IMP,  2, 1 };
    _opcodeTable[0x06] = InstructionData{ "ASL", "ZPG",  &CPU::ASL, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x16] = InstructionData{ "ASL", "ZPGX", &CPU::ASL, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x0E] = InstructionData{ "ASL", "ABS",  &CPU::ASL, &CPU::ABS,  6, 3 };
    _opcodeTable[0x1E] = InstructionData{ "ASL", "ABSX", &CPU::ASL, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x4A] = InstructionData{ "LSR", "IMP",  &CPU::LSR, &CPU::IMP,  2, 1 };
    _opcodeTable[0x46] = InstructionData{ "LSR", "ZPG",  &CPU::LSR, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x56] = InstructionData{ "LSR", "ZPGX", &CPU::LSR, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x4E] = InstructionData{ "LSR", "ABS",  &CPU::LSR, &CPU::ABS,  6, 3 };
    _opcodeTable[0x5E] = InstructionData{ "LSR", "ABSX", &CPU::LSR, &CPU::ABSX, 7, 3, false, true };

    // ROL, ROR
    _opcodeTable[0x2A] = InstructionData{ "ROL", "IMP",  &CPU::ROL, &CPU::IMP,  2, 1 };
    _opcodeTable[0x26] = InstructionData{ "ROL", "ZPG",  &CPU::ROL, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x36] = InstructionData{ "ROL", "ZPGX", &CPU::ROL, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x2E] = InstructionData{ "ROL", "ABS",  &CPU::ROL, &CPU::ABS,  6, 3 };
    _opcodeTable[0x3E] = InstructionData{ "ROL", "ABSX", &CPU::ROL, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x6A] = InstructionData{ "ROR", "IMP",  &CPU::ROR, &CPU::IMP,  2, 1 };
    _opcodeTable[0x66] = InstructionData{ "ROR", "ZPG",  &CPU::ROR, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x76] = InstructionData{ "ROR", "ZPGX", &CPU::ROR, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x6E] = InstructionData{ "ROR", "ABS",  &CPU::ROR, &CPU::ABS,  6, 3 };
    _opcodeTable[0x7E] = InstructionData{ "ROR", "ABSX", &CPU::ROR, &CPU::ABSX, 7, 3, false, true };

    // JMP JSR, RTS, RTI, BRK
    _opcodeTable[0x4C] = InstructionData{ "JMP", "ABS",  &CPU::JMP, &CPU::ABS,  3, 3 };
    _opcodeTable[0x6C] = InstructionData{ "JMP", "IND",  &CPU::JMP, &CPU::IND,  5, 3 };
    _opcodeTable[0x20] = InstructionData{ "JSR", "ABS",  &CPU::JSR, &CPU::ABS,  6, 3 };
    _opcodeTable[0x60] = InstructionData{ "RTS", "IMP",  &CPU::RTS, &CPU::IMP,  6, 1 };
    _opcodeTable[0x00] = InstructionData{ "BRK", "IMP",  &CPU::BRK, &CPU::IMP,  7, 1 };
    _opcodeTable[0x40] = InstructionData{ "RTI", "IMP",  &CPU::RTI, &CPU::IMP,  6, 1 };

    // AND
    _opcodeTable[0x29] = InstructionData{ "AND", "IMM",  &CPU::AND, &CPU::IMM,  2, 2 };
    _opcodeTable[0x25] = InstructionData{ "AND", "ZPG",  &CPU::AND, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x35] = InstructionData{ "AND", "ZPGX", &CPU::AND, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x2D] = InstructionData{ "AND", "ABS",  &CPU::AND, &CPU::ABS,  4, 3 };
    _opcodeTable[0x3D] = InstructionData{ "AND", "ABSX", &CPU::AND, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x39] = InstructionData{ "AND", "ABSY", &CPU::AND, &CPU::ABSY, 4, 3 };
    _opcodeTable[0x21] = InstructionData{ "AND", "INDX", &CPU::AND, &CPU::INDX, 6, 2 };
    _opcodeTable[0x31] = InstructionData{ "AND", "INDY", &CPU::AND, &CPU::INDY, 5, 2 };

    // ORA
    _opcodeTable[0x09] = InstructionData{ "ORA", "IMM",  &CPU::ORA, &CPU::IMM,  2, 2 };
    _opcodeTable[0x05] = InstructionData{ "ORA", "ZPG",  &CPU::ORA, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x15] = InstructionData{ "ORA", "ZPGX", &CPU::ORA, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x0D] = InstructionData{ "ORA", "ABS",  &CPU::ORA, &CPU::ABS,  4, 3 };
    _opcodeTable[0x1D] = InstructionData{ "ORA", "ABSX", &CPU::ORA, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x19] = InstructionData{ "ORA", "ABSY", &CPU::ORA, &CPU::ABSY, 4, 3 };
    _opcodeTable[0x01] = InstructionData{ "ORA", "INDX", &CPU::ORA, &CPU::INDX, 6, 2 };
    _opcodeTable[0x11] = InstructionData{ "ORA", "INDY", &CPU::ORA, &CPU::INDY, 5, 2 };

    // EOR
    _opcodeTable[0x49] = InstructionData{ "EOR", "IMM",  &CPU::EOR, &CPU::IMM,  2, 2 };
    _opcodeTable[0x45] = InstructionData{ "EOR", "ZPG",  &CPU::EOR, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x55] = InstructionData{ "EOR", "ZPGX", &CPU::EOR, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x4D] = InstructionData{ "EOR", "ABS",  &CPU::EOR, &CPU::ABS,  4, 3 };
    _opcodeTable[0x5D] = InstructionData{ "EOR", "ABSX", &CPU::EOR, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x59] = InstructionData{ "EOR", "ABSY", &CPU::EOR, &CPU::ABSY, 4, 3 };
    _opcodeTable[0x41] = InstructionData{ "EOR", "INDX", &CPU::EOR, &CPU::INDX, 6, 2 };
    _opcodeTable[0x51] = InstructionData{ "EOR", "INDY", &CPU::EOR, &CPU::INDY, 5, 2 };

    // BIT
    _opcodeTable[0x24] = InstructionData{ "BIT", "ZPG",  &CPU::BIT, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x2C] = InstructionData{ "BIT", "ABS",  &CPU::BIT, &CPU::ABS,  4, 3 };

    // Transfer
    _opcodeTable[0xAA] = InstructionData{ "TAX", "IMP",  &CPU::TAX, &CPU::IMP,  2, 1 };
    _opcodeTable[0x8A] = InstructionData{ "TXA", "IMP",  &CPU::TXA, &CPU::IMP,  2, 1 };
    _opcodeTable[0xA8] = InstructionData{ "TAY", "IMP",  &CPU::TAY, &CPU::IMP,  2, 1 };
    _opcodeTable[0x98] = InstructionData{ "TYA", "IMP",  &CPU::TYA, &CPU::IMP,  2, 1 };

    /*
    ################################
    ||       Illegal Opcodes      ||
    ################################
    */
    // Jams (does nothing)
    _opcodeTable[0x02] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x12] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x22] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x32] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x42] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x52] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x62] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x72] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0x92] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0xB2] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0xD2] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };
    _opcodeTable[0xF2] = InstructionData{ "*JAM", "IMP", &CPU::JAM, &CPU::IMP,  3, 1 };

    // NOP Implied
    _opcodeTable[0x1A] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };
    _opcodeTable[0x3A] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };
    _opcodeTable[0x5A] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };
    _opcodeTable[0x7A] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };
    _opcodeTable[0xDA] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };
    _opcodeTable[0xFA] = InstructionData{ "*NOP", "IMP", &CPU::NOP, &CPU::IMP,  2, 1 };

    // NOP Immediate
    _opcodeTable[0x80] = InstructionData{ "*NOP", "IMM", &CPU::NOP2, &CPU::IMM, 2, 2 };
    _opcodeTable[0x82] = InstructionData{ "*NOP", "IMM", &CPU::NOP2, &CPU::IMM, 2, 2 };
    _opcodeTable[0x89] = InstructionData{ "*NOP", "IMM", &CPU::NOP2, &CPU::IMM, 2, 2 };
    _opcodeTable[0xC2] = InstructionData{ "*NOP", "IMM", &CPU::NOP2, &CPU::IMM, 2, 2 };
    _opcodeTable[0xE2] = InstructionData{ "*NOP", "IMM", &CPU::NOP2, &CPU::IMM, 2, 2 };

    // NOP Zero Page
    _opcodeTable[0x04] = InstructionData{ "*NOP", "ZPG", &CPU::NOP2, &CPU::ZPG, 3, 2 };
    _opcodeTable[0x44] = InstructionData{ "*NOP", "ZPG", &CPU::NOP2, &CPU::ZPG, 3, 2 };
    _opcodeTable[0x64] = InstructionData{ "*NOP", "ZPG", &CPU::NOP2, &CPU::ZPG, 3, 2 };

    // NOP Zero Page X
    _opcodeTable[0x14] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x34] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x54] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0x74] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xD4] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };
    _opcodeTable[0xF4] = InstructionData{ "*NOP", "ZPGX", &CPU::NOP2, &CPU::ZPGX, 4, 2 };

    // NOP Absolute
    _opcodeTable[0x0C] = InstructionData{ "*NOP", "ABS",  &CPU::NOP2, &CPU::ABS,  4, 3 };
    _opcodeTable[0x1C] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x3C] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x5C] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };
    _opcodeTable[0x7C] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };
    _opcodeTable[0xDC] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };
    _opcodeTable[0xFC] = InstructionData{ "*NOP", "ABSX", &CPU::NOP2, &CPU::ABSX, 4, 3 };

    // SLO
    _opcodeTable[0x07] = InstructionData{ "*SLO", "ZPG",  &CPU::SLO, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x17] = InstructionData{ "*SLO", "ZPGX", &CPU::SLO, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x0F] = InstructionData{ "*SLO", "ABS",  &CPU::SLO, &CPU::ABS,  6, 3 };
    _opcodeTable[0x1F] = InstructionData{ "*SLO", "ABSX", &CPU::SLO, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x1B] = InstructionData{ "*SLO", "ABSY", &CPU::SLO, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0x03] = InstructionData{ "*SLO", "INDX", &CPU::SLO, &CPU::INDX, 8, 2 };
    _opcodeTable[0x13] = InstructionData{ "*SLO", "INDY", &CPU::SLO, &CPU::INDY, 8, 2, false, true };

    // RLA
    _opcodeTable[0x27] = InstructionData{ "*RLA", "ZPG",  &CPU::RLA, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x37] = InstructionData{ "*RLA", "ZPGX", &CPU::RLA, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x2F] = InstructionData{ "*RLA", "ABS",  &CPU::RLA, &CPU::ABS,  6, 3 };
    _opcodeTable[0x3F] = InstructionData{ "*RLA", "ABSX", &CPU::RLA, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x3B] = InstructionData{ "*RLA", "ABSY", &CPU::RLA, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0x23] = InstructionData{ "*RLA", "INDX", &CPU::RLA, &CPU::INDX, 8, 2 };
    _opcodeTable[0x33] = InstructionData{ "*RLA", "INDY", &CPU::RLA, &CPU::INDY, 8, 2, false, true };

    // SRE
    _opcodeTable[0x47] = InstructionData{ "*SRE", "ZPG",  &CPU::SRE, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x57] = InstructionData{ "*SRE", "ZPGX", &CPU::SRE, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x4F] = InstructionData{ "*SRE", "ABS",  &CPU::SRE, &CPU::ABS,  6, 3 };
    _opcodeTable[0x5F] = InstructionData{ "*SRE", "ABSX", &CPU::SRE, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x5B] = InstructionData{ "*SRE", "ABSY", &CPU::SRE, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0x43] = InstructionData{ "*SRE", "INDX", &CPU::SRE, &CPU::INDX, 8, 2 };
    _opcodeTable[0x53] = InstructionData{ "*SRE", "INDY", &CPU::SRE, &CPU::INDY, 8, 2, false, true };

    // RRA
    _opcodeTable[0x67] = InstructionData{ "*RRA", "ZPG",  &CPU::RRA, &CPU::ZPG,  5, 2 };
    _opcodeTable[0x77] = InstructionData{ "*RRA", "ZPGX", &CPU::RRA, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0x6F] = InstructionData{ "*RRA", "ABS",  &CPU::RRA, &CPU::ABS,  6, 3 };
    _opcodeTable[0x7F] = InstructionData{ "*RRA", "ABSX", &CPU::RRA, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0x7B] = InstructionData{ "*RRA", "ABSY", &CPU::RRA, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0x63] = InstructionData{ "*RRA", "INDX", &CPU::RRA, &CPU::INDX, 8, 2 };
    _opcodeTable[0x73] = InstructionData{ "*RRA", "INDY", &CPU::RRA, &CPU::INDY, 8, 2, false, true };

    // SAX
    _opcodeTable[0x87] = InstructionData{ "*SAX", "ZPG",  &CPU::SAX, &CPU::ZPG,  3, 2 };
    _opcodeTable[0x97] = InstructionData{ "*SAX", "ZPGY", &CPU::SAX, &CPU::ZPGY, 4, 2, true, true };
    _opcodeTable[0x8F] = InstructionData{ "*SAX", "ABS",  &CPU::SAX, &CPU::ABS,  4, 3 };
    _opcodeTable[0x83] = InstructionData{ "*SAX", "INDX", &CPU::SAX, &CPU::INDX, 6, 2 };

    // LAX
    _opcodeTable[0xA7] = InstructionData{ "*LAX", "ZPG",  &CPU::LAX, &CPU::ZPG,  3, 2 };
    _opcodeTable[0xB7] = InstructionData{ "*LAX", "ZPGY", &CPU::LAX, &CPU::ZPGY, 4, 2, true, true };
    _opcodeTable[0xAF] = InstructionData{ "*LAX", "ABS",  &CPU::LAX, &CPU::ABS,  4, 3 };
    _opcodeTable[0xBF] = InstructionData{ "*LAX", "ABSY", &CPU::LAX, &CPU::ABSY, 4, 3 };
    _opcodeTable[0xA3] = InstructionData{ "*LAX", "INDX", &CPU::LAX, &CPU::INDX, 6, 2 };
    _opcodeTable[0xB3] = InstructionData{ "*LAX", "INDY", &CPU::LAX, &CPU::INDY, 5, 2 };

    // DCP
    _opcodeTable[0xC7] = InstructionData{ "*DCP", "ZPG",  &CPU::DCP, &CPU::ZPG,  5, 2 };
    _opcodeTable[0xD7] = InstructionData{ "*DCP", "ZPGX", &CPU::DCP, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0xCF] = InstructionData{ "*DCP", "ABS",  &CPU::DCP, &CPU::ABS,  6, 3 };
    _opcodeTable[0xDF] = InstructionData{ "*DCP", "ABSX", &CPU::DCP, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0xDB] = InstructionData{ "*DCP", "ABSY", &CPU::DCP, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0xC3] = InstructionData{ "*DCP", "INDX", &CPU::DCP, &CPU::INDX, 8, 2 };
    _opcodeTable[0xD3] = InstructionData{ "*DCP", "INDY", &CPU::DCP, &CPU::INDY, 8, 2, false, true };

    // ISC
    _opcodeTable[0xE7] = InstructionData{ "*ISC", "ZPG",  &CPU::ISC, &CPU::ZPG,  5, 2 };
    _opcodeTable[0xF7] = InstructionData{ "*ISC", "ZPGX", &CPU::ISC, &CPU::ZPGX, 6, 2 };
    _opcodeTable[0xEF] = InstructionData{ "*ISC", "ABS",  &CPU::ISC, &CPU::ABS,  6, 3 };
    _opcodeTable[0xFF] = InstructionData{ "*ISC", "ABSX", &CPU::ISC, &CPU::ABSX, 7, 3, false, true };
    _opcodeTable[0xFB] = InstructionData{ "*ISC", "ABSY", &CPU::ISC, &CPU::ABSY, 7, 3, false, true };
    _opcodeTable[0xE3] = InstructionData{ "*ISC", "INDX", &CPU::ISC, &CPU::INDX, 8, 2 };
    _opcodeTable[0xF3] = InstructionData{ "*ISC", "INDY", &CPU::ISC, &CPU::INDY, 8, 2, false, true };

    // SBC2
    _opcodeTable[0xEB] = InstructionData{ "*SBC", "IMM",  &CPU::SBC, &CPU::IMM,  2, 2 };

    // ALR, ARR
    _opcodeTable[0x4B] = InstructionData{ "*ALR", "IMM",  &CPU::ALR, &CPU::IMM,  2, 2 };
    _opcodeTable[0x6B] = InstructionData{ "*ARR", "IMM",  &CPU::ARR, &CPU::IMM,  2, 2 };

    // ANC
    _opcodeTable[0x0B] = InstructionData{ "*ANC", "IMM",  &CPU::ANC, &CPU::IMM,  2, 2 };
    _opcodeTable[0x2B] = InstructionData{ "*ANC", "IMM",  &CPU::ANC, &CPU::IMM,  2, 2 };

    // LXA
    _opcodeTable[0xAB] = InstructionData{ "*LXA", "IMM",  &CPU::LXA, &CPU::IMM,  2, 2 };

    // SBX
    _opcodeTable[0xCB] = InstructionData{ "*SBX", "IMM",  &CPU::SBX, &CPU::IMM,  2, 2 };
    // clang-format on
    // NOLINTEND

    /*
    ################################
    ||         Validation         ||
    ################################
    */
    // Validate Opcode names and address mode strings
    for ( int i = 0; i < 256; i++ )
    {
        string const name = _opcodeTable[i].name;
        // if no name, skip, it's an empty entry
        if ( name.empty() )
        {
            continue;
        }
        string const addr_mode = _opcodeTable[i].addr_mode;
        if ( !utils::isValidOpcodeName( name ) )
        {
            string output = "Invalid opcode name: " + name;
            output += " for opcode: " + utils::toHex( i, 2 );
            output += '\n';
            output += "Valid names: \n" + utils::getAvailableOpcodeNames();
            throw std::runtime_error( output );
        }
        if ( !utils::isValidAddrModeStr( addr_mode ) )
        {
            string output = "Invalid address mode string: " + addr_mode;
            output += " for opcode: " + utils::toHex( i, 2 );
            output += '\n';
            output += "Valid address mode strings: \n" + utils::getAvailableAddrModes();
            throw std::runtime_error( output );
        };
    }
};

/*
################################################
||                                            ||
||                   Getters                  ||
||                                            ||
################################################
*/
[[nodiscard]] u8  CPU::GetAccumulator() const { return _a; }
[[nodiscard]] u8  CPU::GetXRegister() const { return _x; }
[[nodiscard]] u8  CPU::GetYRegister() const { return _y; }
[[nodiscard]] u8  CPU::GetStatusRegister() const { return _p; }
[[nodiscard]] u16 CPU::GetProgramCounter() const { return _pc; }
[[nodiscard]] u8  CPU::GetStackPointer() const { return _s; }
[[nodiscard]] u64 CPU::GetCycles() const { return _cycles; }

/*
################################################
||                                            ||
||                   Setters                  ||
||                                            ||
################################################
*/
void CPU::SetAccumulator( u8 value ) { _a = value; }
void CPU::SetXRegister( u8 value ) { _x = value; }
void CPU::SetYRegister( u8 value ) { _y = value; }
void CPU::SetStatusRegister( u8 value ) { _p = value; }
void CPU::SetProgramCounter( u16 value ) { _pc = value; }
void CPU::SetStackPointer( u8 value ) { _s = value; }
void CPU::SetCycles( u64 value ) { _cycles = value; }

/*
################################################
||                                            ||
||                Debug Methods               ||
||                                            ||
################################################
*/
[[nodiscard]] std::string CPU::GetTrace() const { return _trace; }

void CPU::EnableTracelog() { _trace_enabled = true; }
void CPU::DisableTracelog() { _trace_enabled = false; }

std::string CPU::LogLineAtPC( bool verbose ) // NOLINT
{
    /*
     * @brief Disassembles the instruction at the current program counter
     * Useful to understand what the current instruction is doing
     */
    std::string output;

    std::string       name = _opcodeTable[Read( _pc )].name;
    std::string const addr_mode = _opcodeTable[Read( _pc )].addr_mode;

    // Program counter address
    // i.e. FFFF
    output += utils::toHex( _pc, 4 ) + ":  ";

    // Hex instruction
    // i.e. 4C F5 C5, this is the hex instruction
    u8 const    bytes = _opcodeTable[Read( _pc )].bytes;
    std::string hex_instruction;
    for ( u8 i = 0; i < bytes; i++ )
    {
        hex_instruction += utils::toHex( Read( _pc + i ), 2 ) + ' ';
    }

    // formatting, the instruction hex_instruction will be 9 characters long, with space padding to
    // the right. This makes sure the hex line is the same length for all instructions
    hex_instruction += std::string( 9 - ( bytes * 3 ), ' ' );
    output += hex_instruction;

    // If name starts with a "*", it is an illegal opcode
    ( name[0] == '*' ) ? output += "*" + name.substr( 1 ) + " " : output += name + " ";

    // Addressing mode and operand

    std::string assembly_str;
    u8          value = 0x00;
    u8          low = 0x00;
    u8          high = 0x00;
    if ( addr_mode == "IMP" )
    {
        // Nothing to prefix
    }
    else if ( addr_mode == "IMM" )
    {
        value = Read( _pc + 1 );
        assembly_str += "#$" + utils::toHex( value, 2 );
    }
    else if ( addr_mode == "ZPG" || addr_mode == "ZPGX" || addr_mode == "ZPGY" )
    {
        value = Read( _pc + 1 );
        assembly_str += "$" + utils::toHex( value, 2 );

        ( addr_mode == "ZPGX" )   ? assembly_str += ", X"
        : ( addr_mode == "ZPGY" ) ? assembly_str += ", Y"
                                  : assembly_str += "";
    }
    else if ( addr_mode == "ABS" || addr_mode == "ABSX" || addr_mode == "ABSY" )
    {
        low = Read( _pc + 1 );
        high = Read( _pc + 2 );
        u16 const address = ( high << 8 ) | low;

        assembly_str += "$" + utils::toHex( address, 4 );
        ( addr_mode == "ABSX" )   ? assembly_str += ", X"
        : ( addr_mode == "ABSY" ) ? assembly_str += ", Y"
                                  : assembly_str += "";
    }
    else if ( addr_mode == "IND" )
    {
        low = Read( _pc + 1 );
        high = Read( _pc + 2 );
        u16 const address = ( high << 8 ) | low;
        assembly_str += "($" + utils::toHex( address, 4 ) + ")";
    }
    else if ( addr_mode == "INDX" || addr_mode == "INDY" )
    {
        value = Read( _pc + 1 );
        ( addr_mode == "INDX" ) ? assembly_str += "($" + utils::toHex( value, 2 ) + ", X)"
                                : assembly_str += "($" + utils::toHex( value, 2 ) + "), Y";
    }
    else if ( addr_mode == "REL" )
    {
        value = Read( _pc + 1 );
        s8 const  offset = static_cast<s8>( value );
        u16 const address = _pc + 2 + offset;

        assembly_str += "$" + utils::toHex( value, 2 ) + " [$" + utils::toHex( address, 4 ) + "]";
    }
    else
    {
        // Houston.. yet again
        throw std::runtime_error( "Unknown addressing mode: " + addr_mode );
    }

    // Pad the assembly string with spaces, for fixed length
    output += assembly_str + std::string( 15 - assembly_str.size(), ' ' );

    // Add more log info
    if ( verbose )
    {
        std::string registers_str;
        // Format
        // a: 00 x: 00 y: 00 s: FD
        registers_str += "a: " + utils::toHex( _a, 2 ) + " ";
        registers_str += "x: " + utils::toHex( _x, 2 ) + " ";
        registers_str += "y: " + utils::toHex( _y, 2 ) + " ";
        registers_str += "s: " + utils::toHex( _s, 2 ) + " ";

        // status register
        // Will return a formatted status string
        // p: hex value, status string (NV-BDIZC). Letter present is flag set, dash is flag unset
        std::string status_str;
        status_str += "p: " + utils::toHex( _p, 2 ) + "  ";

        std::string status_flags = "NV-BDIZC";
        std::string status_flags_lower = "nv--dizc";
        std::string status_flags_str;
        for ( int i = 7; i >= 0; i-- )
        {
            status_flags_str += ( _p & ( 1 << i ) ) != 0 ? status_flags[7 - i] : status_flags_lower[7 - i];
        }
        status_str += status_flags_str;

        // Combine to the output string
        output += registers_str + status_str;

        // Scanline num (V)
        std::string const scanline_str = std::to_string( _bus->ppu.GetScanline() );
        // std::string scanline_str_adjusted = std::string( 4 - scanline_str.size(), ' ' );
        output += "  V: " + scanline_str;

        // PPU cycles (H), pad for 3 characters + space
        u16 const   ppu_cycles = _bus->ppu.GetCycles();
        std::string ppu_cycles_str = std::to_string( ppu_cycles );
        ppu_cycles_str += std::string( 4 - ppu_cycles_str.size(), ' ' );
        output += "  H: " + ppu_cycles_str; // PPU cycle

        // cycle count
        output += "  Cycle: " + std::to_string( _cycles );
    }

    return output;
}

/*
################################################################
||                                                            ||
||                        CPU Methods                         ||
||                                                            ||
################################################################
*/

// Pass off reads and writes to the bus
auto CPU::Read( u16 address ) const -> u8 { return _bus->Read( address ); }
void CPU::Write( u16 address, u8 data ) const { _bus->Write( address, data ); }

// Read with cycle spend
auto CPU::ReadAndTick( u16 address ) -> u8
{
    if ( address == 0x2002 )
    {
        _bus->ppu.SetIsCpuReadingPpuStatus( true );
    }
    Tick();
    u8 const data = Read( address );
    return data;
}

// Write and spend a cycle
auto CPU::WriteAndTick( u16 address, u8 data ) -> void
{
    Tick();

    // Writing to PPUCTRL, PPUMASK, PPUSCROLL, and PPUADDR is ignored until after cycle ~29658
    if ( !_bus->IsTestMode() &&
         ( address == 0x2000 || address == 0x2001 || address == 0x2005 || address == 0x2006 ) )
    {
        if ( _cycles < 29658 )
        {
            return;
        }
    }
    Write( address, data );
}

u8 CPU::Fetch()
{

    // Read the current PC location and increment it

    u8 const opcode = ReadAndTick( _pc++ );
    return opcode;
}

void CPU::Tick()
{
    // Increment the cycle count
    _cycles++;
    _bus->ppu.Tick();
    _bus->ppu.Tick();

    /*
       Saves cpu state after 2 ppu cycles of the first cpu cycle.
       Used for creating a trace log that matches Mesen. This is
       a costly operation, so it's only used for debugging
     */
    if ( !_did_trace && _trace_enabled )
    {
        _pc--;
        _trace = LogLineAtPC( true );
        _pc++;
        _did_trace = true;
    }

    _bus->ppu.Tick();
}

void CPU::Reset()
{
    _a = 0x00;
    _x = 0x00;
    _y = 0x00;
    _s = 0xFD;
    _p = 0x00 | Unused | InterruptDisable;

    // The program counter is usually read from the reset vector of a game, which is
    // located at 0xFFFC and 0xFFFD. If no cartridge, we'll assume 0x00 for both
    _pc = Read( 0xFFFD ) << 8 | Read( 0xFFFC );

    // Add 7 cycles
    if ( !_bus->IsTestMode() )
    {

        for ( u8 i = 0; i < 7; i++ )
        {
            Tick();
        }
    }
    else
    {
        _cycles = 0;
    }
}

/**
 * @brief Executes a single CPU cycle.
 *
 * This function fetches the next opcode from memory, decodes it using the opcode table,
 * and executes that instruction. It also adds the number of cycles the instruction
 * takes to the total cycle count.
 *
 * If the opcode is invalid, an error message is printed to stderr.
 */

void CPU::NMI()
{
    /* @details: Non-maskable Interrupt, called by the PPU during the VBlank period.
     * It interrupts whatever the CPU is doing at its current cycle to go update the PPU.
     * Uses 7 cycles, cannot be disabled.
     */
    // 1) Two dummy cycles (hardware reads the same PC twice, discarding the data)
    Tick();
    Tick();

    // 2) Push PC high, then PC low
    StackPush( ( _pc >> 8 ) & 0xFF );
    StackPush( _pc & 0xFF );

    // 3) Push status register with B=0; bit 5 (Unused) = 1
    u8 const pushed_status = ( _p & ~Break ) | Unused;
    StackPush( pushed_status );

    // 4) Fetch low byte of NMI vector ($FFFA)
    u8 const low = ReadAndTick( 0xFFFA );

    // 5) Set I flag
    SetFlags( InterruptDisable );

    // 6) Fetch high byte of NMI vector ($FFFB)
    u8 const high = ReadAndTick( 0xFFFB );

    // 7) Update PC
    _pc = static_cast<u16>( high ) << 8 | low;
}

void CPU::IRQ()
{
    /* @brief: IRQ, can be called when interrupt disable is turned off.
     * Uses 7 cycles
     */
    if ( ( _p & InterruptDisable ) != 0 )
    {
        return;
    }
    Tick();
    Tick();
    StackPush( ( _pc >> 8 ) & 0xFF );
    StackPush( _pc & 0xFF );
    u8 const pushed_status = ( _p & ~Break ) | Unused;
    StackPush( pushed_status );
    u8 const low = ReadAndTick( 0xFFFE );
    SetFlags( InterruptDisable );
    u8 const high = ReadAndTick( 0xFFFF );
    _pc = static_cast<u16>( high ) << 8 | low;
}

void CPU::DecodeExecute()
{

    /**
     * @brief Decode and execute an instruction
     *
     * This function fetches the next opcode from memory, decodes it using the opcode table,
     * and executes that instruction.
     *
     * If the opcode is invalid, an error message is printed to stderr.
     */

    _did_trace = false;

    // Fetch the next opcode and increment the program counter
    u8 const opcode = Fetch();

    // Decode the opcode
    auto const &instruction = _opcodeTable[opcode];
    auto        instruction_handler = instruction.instructionMethod;
    auto        addressing_mode_handler = instruction.addressingModeMethod;

    if ( instruction_handler != nullptr && addressing_mode_handler != nullptr )
    {
        // Set the page cross penalty for the current instruction
        // Used in addressing modes: ABSX, ABSY, INDY
        _currentPageCrossPenalty = instruction.pageCrossPenalty;

        // Write / modify instructions use a dummy read before writing, so
        // we should set a flag for those
        _is_write_modify = instruction.isWriteModify;

        // Set current instr mnemonic globally
        _instruction_name = instruction.name;

        // Set current address mode string globally
        _addr_mode = instruction.addr_mode;

        // Calculate the address using the addressing mode
        u16 const address = ( this->*addressing_mode_handler )();

        // Execute the instruction fetched from the opcode table
        ( this->*instruction_handler )( address );

        // Reset flags
        _is_write_modify = false;
        _did_trace = false;
    }
    else
    {
        // Houston, we have a problem. No opcode was found.
        std::cerr << "Bad opcode: " << std::hex << static_cast<int>( opcode ) << '\n';
    }
}
/*
################################################################
||                                                            ||
||                      Addressing Modes                      ||
||                                                            ||
################################################################
*/

auto CPU::IMP() -> u16
{
    /*
     * @brief Implicit addressing mode
     * This mode does not require an operand
     */
    Tick();
    return 0;
}

auto CPU::IMM() -> u16
{
    /*
     * @brief Returns address of the next byte in memory (the operand itself)
     * The operand is a part of the instruction
     * The program counter is incremented to point to the operand
     */
    return _pc++;
}

auto CPU::ZPG() -> u16
{
    /*
     * @brief Zero Page addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF).
     * The value of the next byte is the address in the zero page.
     */
    return ReadAndTick( _pc++ ) & 0x00FF;
}

auto CPU::ZPGX() -> u16
{
    /*
     * @brief Zero Page X addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + X register
     * The value of the next byte is the address in the zero page.
     */
    u8 const  zero_page_address = ReadAndTick( _pc++ );
    u16 const final_address = ( zero_page_address + _x ) & 0x00FF;
    Tick(); // Account for calculating the final address
    return final_address;
}

auto CPU::ZPGY() -> u16
{
    /*
     * @brief Zero Page Y addressing mode
     * Returns the address from the zero page (0x0000 - 0x00FF) + Y register
     * The value of the next byte is the address in the zero page.
     */
    u8 const zero_page_address = ( ReadAndTick( _pc++ ) + _y ) & 0x00FF;

    if ( _is_write_modify )
    {
        Tick();
    }
    return zero_page_address;
}

auto CPU::ABS() -> u16
{
    /*
     * @brief Absolute addressing mode
     * Constructs a 16-bit address from the next two bytes
     */
    u16 const low = ReadAndTick( _pc++ );
    u16 const high = ReadAndTick( _pc++ );
    return ( high << 8 ) | low;
}

auto CPU::ABSX() -> u16
{
    /*
     * @brief Absolute X addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the X register to the final
     * address
     */
    u16 const low = ReadAndTick( _pc++ );
    u16 const high = ReadAndTick( _pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const final_address = address + _x;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: ASL, ROL, LSR, ROR, STA, DEC, INC
    if ( _currentPageCrossPenalty && ( final_address & 0xFF00 ) != ( address & 0xFF00 ) )
    {
        Tick();
    }

    if ( _is_write_modify )
    {
        // Dummy read, in preparation to overwrite the address
        Tick();
    }
    return final_address;
}

auto CPU::ABSY() -> u16
{
    /*
     * @brief Absolute Y addressing mode
     * Constructs a 16-bit address from the next two bytes and adds the Y register to the final
     * address
     */
    u16 const low = ReadAndTick( _pc++ );
    u16 const high = ReadAndTick( _pc++ );
    u16 const address = ( high << 8 ) | low;
    u16 const final_address = address + _y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( _currentPageCrossPenalty && ( final_address & 0xFF00 ) != ( address & 0xFF00 ) )
    {
        Tick();
    }
    if ( _is_write_modify )
    {
        // Dummy read, in preparation to overwrite the address
        Tick();
    }

    return final_address;
}

auto CPU::IND() -> u16
{
    /*
     * @brief Indirect addressing mode
     * This mode implements pointers.
     * The pointer address will be read from the next two bytes
     * The returning value is the address stored at the pointer address
     * There's a hardware bug that prevents the address from crossing a page boundary
     */

    u16 const ptr_low = ReadAndTick( _pc++ );
    u16 const ptr_high = ReadAndTick( _pc++ );
    u16 const ptr = ( ptr_high << 8 ) | ptr_low;

    u8 const address_low = ReadAndTick( ptr );
    u8       address_high; // NOLINT

    // 6502 Bug: If the pointer address wraps around a page boundary (e.g. 0x01FF),
    // the CPU reads the low byte from 0x01FF and the high byte from the start of
    // the same page (0x0100) instead of the start of the next page (0x0200).
    if ( ptr_low == 0xFF )
    {
        address_high = ReadAndTick( ptr & 0xFF00 );
    }
    else
    {
        address_high = ReadAndTick( ptr + 1 );
    }

    return ( address_high << 8 ) | address_low;
}

auto CPU::INDX() -> u16
{
    /*
     * @brief Indirect X addressing mode
     * The next two bytes are a zero-page address
     * X register is added to the zero-page address to get the pointer address
     * Final address is the value stored at the POINTER address
     */
    Tick();                                                                 // Account for operand fetch
    u8 const  zero_page_address = ( ReadAndTick( _pc++ ) + _x ) & 0x00FF;   // 1 cycle
    u16 const ptr_low = ReadAndTick( zero_page_address );                   // 1 cycle
    u16 const ptr_high = ReadAndTick( ( zero_page_address + 1 ) & 0x00FF ); // 1 cycle
    return ( ptr_high << 8 ) | ptr_low;
}

auto CPU::INDY() -> u16
{
    /*
     * @brief Indirect Y addressing mode
     * The next byte is a zero-page address
     * The value stored at the zero-page address is the pointer address
     * The value in the Y register is added to the FINAL address
     */
    u16 const zero_page_address = ReadAndTick( _pc++ );
    u16 const ptr_low = ReadAndTick( zero_page_address );
    u16 const ptr_high = ReadAndTick( ( zero_page_address + 1 ) & 0x00FF );

    u16 const address = ( ( ptr_high << 8 ) | ptr_low ) + _y;

    // If the final address crosses a page boundary, an additional cycle is required
    // Instructions that should ignore this: STA
    if ( _currentPageCrossPenalty && ( address & 0xFF00 ) != ( ptr_high << 8 ) )
    {
        Tick();
    }

    if ( _is_write_modify )
    {
        // Dummy read, in preparation to overwrite the address
        Tick();
    }
    return address;
}

auto CPU::REL() -> u16
{
    /*
     * @brief Relative addressing mode
     * The next byte is a signed offset
     * Sets the program counter between -128 and +127 bytes from the current location
     */
    s8 const  offset = static_cast<s8>( ReadAndTick( _pc++ ) );
    u16 const address = _pc + offset;
    return address;
}

/*
################################################################
||                                                            ||
||                    Instruction Helpers                     ||
||                                                            ||
################################################################
*/

void CPU::LoadRegister( u16 address, u8 &reg )
{
    /*
     * @brief It loads a register with a value from memory
     * Used by LDA, LDX, and LDY instructions
     */
    u8 const value = ReadAndTick( address );
    reg = value;

    // Set zero and negative flags
    SetZeroAndNegativeFlags( value );
};

void CPU::StoreRegister( u16 address, u8 reg )
{
    /*
     * @brief It stores a register value in memory
     * Used by STA, STX, and STY instructions
     */
    WriteAndTick( address, reg );
};

void CPU::SetFlags( const u8 flag )
{
    /*
     * @brief set one or more flag bits through bitwise OR with the status register
     *
     * Used by the SEC, SED, and SEI instructions to set one or more flag bits through
     * bitwise OR with the status register.
     *
     * Usage:
     * SetFlags( Status::Carry ); // Set one flag
     * SetFlags( Status::Carry | Status::Zero ); // Set multiple flags
     */
    _p |= flag;
}
void CPU::ClearFlags( const u8 flag )
{
    /* Clear Flags
     * @brief clear one or more flag bits through bitwise AND of the complement (inverted) flag
     * with the status register
     *
     * Used by the CLC, CLD, and CLI instructions to clear one or more flag bits through
     * bitwise AND of the complement (inverted) flag with the status register.
     *
     * Usage:
     * ClearFlags( Status::Carry ); // Clear one flag
     * ClearFlags( Status::Carry | Status::Zero ); // Clear multiple flags
     */
    _p &= ~flag;
}
bool CPU::IsFlagSet( const u8 flag ) const
{
    /* @brief Utility function to check if a given status is set in the status register
     *
     * Usage:
     * if ( IsFlagSet( Status::Carry ) )
     * {
     *   // Do something
     * }
     * if ( IsFlagSet( Status::Carry | Status::Zero ) )
     * {
     *   // Do something
     * }
     */
    return ( _p & flag ) == flag;
}

void CPU::SetZeroAndNegativeFlags( u8 value )
{
    /*
     * @brief Sets zero flag if value == 0, or negative flag if value is negative (bit 7 is set)
     */

    // Clear zero and negative flags
    ClearFlags( Status::Zero | Status::Negative );

    // Set zero flag if value is zero
    if ( value == 0 )
    {
        SetFlags( Status::Zero );
    }

    // Set negative flag if bit 7 is set
    if ( ( value & 0b10000000 ) != 0 )
    {
        SetFlags( Status::Negative );
    }
}

void CPU::BranchOnStatus( u16 offsetAddress, u8 flag, bool isSet )
{
    /* @brief Branch if status flag is set or clear
     *
     * Used by branch instructions to branch if a status flag is set or clear.
     *
     * Usage:
     * BranchOnStatus( Status::Carry, true ); // Branch if carry flag is set
     * BranchOnStatus( Status::Zero, false ); // Branch if zero flag is clear
     */

    bool const will_branch = ( _p & flag ) == flag;

    // Path will branch
    if ( will_branch == isSet )
    {
        // Store previous program counter value, used to check boundary crossing
        u16 const prev_pc = _pc;

        // Set _pc to the offset address, calculated by REL addressing mode
        _pc = offsetAddress;

        // +1 cycles because we're taking a branch
        Tick();

        // Add another cycle if page boundary is crossed
        if ( ( _pc & 0xFF00 ) != ( prev_pc & 0xFF00 ) )
        {
            Tick();
        }
    }
    // Path will not branch, nothing to do
}

void CPU::CompareAddressWithRegister( u16 address, u8 reg )
{
    /*
     * @brief Compare a value in memory with a register
     * Used by CMP, CPX, and CPY instructions
     */

    u8 value = 0;
    if ( _instruction_name == "*DCP" )
    {
        value = Read( address ); // 0 cycles
    }
    else
    {
        value = ReadAndTick( address );
    }

    // Set the zero flag if the values are equal
    ( reg == value ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Set negative flag if the result is negative,
    // i.e. the sign bit is set
    ( ( reg - value ) & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Set the carry flag if the reg >= value
    ( reg >= value ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
}

void CPU::StackPush( u8 value )
{
    /*
     * @brief Push a value onto the stack
     * The stack pointer is decremented and the value is written to the stack
     * Stack addresses are between 0x0100 and 0x01FF
     */
    WriteAndTick( 0x0100 + _s--, value );
}

u8 CPU::StackPop()
{
    /*
     * @brief Pop a value from the stack
     * The stack pointer is incremented and the value is read from the stack
     * Stack addresses are between 0x0100 and 0x01FF
     */
    return ReadAndTick( 0x0100 + ++_s );
}

/*
################################################################
||                                                            ||
||                        Instructions                        ||
||                                                            ||
################################################################
* These functions should take no arguments and return no values.
* All complicated or reusable logic should be defined in the helper
* methods.
*/

void CPU::NOP( u16 address ) // NOLINT
{
    /*
     * @brief No operation
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * NOP Implied: EA(2)
     *
     * --  Illegal  --
     * NOP Implied: 1A(2)
     * NOP Implied: 3A(2)
     * NOP Implied: 5A(2)
     * NOP Implied: 7A(2)
     * NOP Implied: DA(2)
     * NOP Implied: FA(2)
     * NOP Immediate: 80(2)
     * NOP Immediate: 82(2)
     * NOP Immediate: 89(2)
     * NOP Immediate: C2(2)
     * NOP Immediate: E2(2)
     * NOP Zero Page: 04(3)
     * NOP Zero Page: 44(3)
     * NOP Zero Page: 64(3)
     * NOP Zero Page X: 14(4)
     * NOP Zero Page X: 34(4)
     * NOP Zero Page X: 54(4)
     * NOP Zero Page X: 74(4)
     * NOP Zero Page X: D4(4)
     * NOP Zero Page X: F4(4)
     * NOP Absolute: 0C(4)
     * NOP Absolute: 1C(4)
     * NOP Absolute: 3C(4)
     * NOP Absolute: 5C(4)
     * NOP Absolute: 7C(4)
     * NOP Absolute: DC(4)
     * NOP Absolute: FC(4)
     */
    (void) address;
}

void CPU::LDA( u16 address )
{
    /*
     * @brief Load Accumulator with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDA Immediate: A9(2)
     * LDA Zero Page: A5(3)
     * LDA Zero Page X: B5(4)
     * LDA Absolute: AD(4)
     * LDA Absolute X: BD(4+)
     * LDA Absolute Y: B9(4+)
     * LDA Indirect X: A1(6)
     * LDA Indirect Y: B1(5+)
     */

    LoadRegister( address, _a );
}

void CPU::LDX( u16 address )
{
    /*
     * @brief Load X Register with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDX Immediate: A2(2)
     * LDX Zero Page: A6(3)
     * LDX Zero Page Y: B6(4)
     * LDX Absolute: AE(4)
     * LDX Absolute Y: BE(4+)
     */
    LoadRegister( address, _x );
}

void CPU::LDY( u16 address )
{
    /*
     * @brief Load Y Register with Memory
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * LDY Immediate: A0(2)
     * LDY Zero Page: A4(3)
     * LDY Zero Page X: B4(4)
     * LDY Absolute: AC(4)
     * LDY Absolute X: BC(4+)
     */
    LoadRegister( address, _y );
}

void CPU::STA( const u16 address ) // NOLINT
{
    /*
     * @brief Store Accumulator in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STA Zero Page: 85(3)
     * STA Zero Page X: 95(4)
     * STA Absolute: 8D(4)
     * STA Absolute X: 9D(5)
     * STA Absolute Y: 99(5)
     * STA Indirect X: 81(6)
     * STA Indirect Y: 91(6)
     */
    StoreRegister( address, _a );
}

void CPU::STX( const u16 address ) // NOLINT
{
    /*
     * @brief Store X Register in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STX Zero Page: 86(3)
     * STX Zero Page Y: 96(4)
     * STX Absolute: 8E(4)
     */
    StoreRegister( address, _x );
}

void CPU::STY( const u16 address ) // NOLINT
{
    /*
     * @brief Store Y Register in Memory
     * N Z C I D V
     * - - - - - -
     * Usage and cycles:
     * STY Zero Page: 84(3)
     * STY Zero Page X: 94(4)
     * STY Absolute: 8C(4)
     */
    StoreRegister( address, _y );
}

void CPU::ADC( u16 address )
{
    /*
     * @brief Add Memory to Accumulator with Carry
     * N Z C I D V
     * + + + - - +
     * Usage and cycles:
     * ADC Immediate: 69(2)
     * ADC Zero Page: 65(3)
     * ADC Zero Page X: 75(4)
     * ADC Absolute: 6D(4)
     * ADC Absolute X: 7D(4+)
     * ADC Absolute Y: 79(4+)
     * ADC Indirect X: 61(6)
     * ADC Indirect Y: 71(5+)
     */
    u8 value = 0;

    if ( _instruction_name == "*RRA" )
    {
        value = Read( address ); // No cycle spend
    }
    else
    {
        value = ReadAndTick( address );
    }

    // Store the sum in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 1 : 0;
    u16 const sum = _a + value + carry;

    // Set the carry flag if sum > 255
    // this means that there will be an overflow
    ( sum > 0xFF ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // If the lower part of sum is zero, set the zero flag
    ( ( sum & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Signed overflow is set if the sign bit is different in the accumulator and the result
    // e.g.
    // 1000 0001 + // << Accumulator: -127
    // 1000 0001   // << Value: -127
    // ---------
    // 0000 0010   // << Sum: 2. Sign bit is different, result is positive but should be
    // negative
    u8 const accumulator_sign_bit = _a & 0b10000000;
    u8 const value_sign_bit = value & 0b10000000;
    u8 const sum_sign_bit = sum & 0b10000000;
    ( accumulator_sign_bit == value_sign_bit && accumulator_sign_bit != sum_sign_bit )
        ? SetFlags( Status::Overflow )
        : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( sum & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the sum in the accumulator
    _a = sum & 0xFF;
}

void CPU::SBC( u16 address )
{
    /* @brief Subtract Memory from Accumulator with Borrow
     * N Z C I D V
     * + + + - - +
     * Usage and cycles:
     * SBC Immediate: E9(2)
     * SBC Zero Page: E5(3)
     * SBC Zero Page X: F5(4)
     * SBC Absolute: ED(4)
     * SBC Absolute X: FD(4+)
     * SBC Absolute Y: F9(4+)
     * SBC Indirect X: E1(6)
     * SBC Indirect Y: F1(5+)
     *
     * --  Illegal  --
     * SBC Immediate: EB(2)
     */

    u8 value = 0;
    if ( _instruction_name == "*ISC" )
    {
        value = Read( address ); // 0 cycles
    }
    else
    {
        value = ReadAndTick( address );
    }
    // u8 const value = ReadAndTick( address );

    // Store diff in a 16-bit variable to check for overflow
    u8 const  carry = IsFlagSet( Status::Carry ) ? 0 : 1;
    u16 const diff = _a - value - carry;

    // Carry flag exists in the high byte?
    ( diff < 0x100 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // If the lower part of diff is zero, set the zero flag
    ( ( diff & 0xFF ) == 0 ) ? SetFlags( Status::Zero ) : ClearFlags( Status::Zero );

    // Signed overflow is set if the sign bit is different in the accumulator and the result
    // e.g.
    // 0000 0001 - // << Accumulator: 1
    // 0000 0010   // << Value: 2
    // ---------
    // 1111 1111   // << Diff: 127. Sign bit is different
    u8 const accumulator_sign_bit = _a & 0b10000000;
    u8 const value_sign_bit = value & 0b10000000;
    u8 const diff_sign_bit = diff & 0b10000000;
    ( accumulator_sign_bit != value_sign_bit && accumulator_sign_bit != diff_sign_bit )
        ? SetFlags( Status::Overflow )
        : ClearFlags( Status::Overflow );

    // If bit 7 is set, set the negative flag
    ( diff & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );

    // Store the lower byte of the diff in the accumulator
    _a = diff & 0xFF;
}

void CPU::INC( u16 address )
{
    /*
     * @brief Increment Memory by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INC Zero Page: E6(5)
     * INC Zero Page X: F6(6)
     * INC Absolute: EE(6)
     * INC Absolute X: FE(7)
     */
    u8 const value = ReadAndTick( address );
    Tick(); // Dummy write
    u8 const result = value + 1;
    SetZeroAndNegativeFlags( result );
    WriteAndTick( address, result );
}

void CPU::INX( u16 address )
{
    /*
     * @brief Increment X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INX: E8(2)
     */
    (void) address;
    _x++;
    SetZeroAndNegativeFlags( _x );
}

void CPU::INY( u16 address )
{
    /*
     * @brief Increment Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * INY: C8(2)
     */
    (void) address;
    _y++;
    SetZeroAndNegativeFlags( _y );
}

void CPU::DEC( u16 address )
{
    /*
     * @brief Decrement Memory by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEC Zero Page: C6(5)
     * DEC Zero Page X: D6(6)
     * DEC Absolute: CE(6)
     * DEC Absolute X: DE(7)
     */
    u8 const value = ReadAndTick( address );
    Tick(); // Dummy write
    u8 const result = value - 1;
    SetZeroAndNegativeFlags( result );
    WriteAndTick( address, result );
}

void CPU::DEX( u16 address )
{
    /*
     * @brief Decrement X Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEX: CA(2)
     */
    (void) address;
    _x--;
    SetZeroAndNegativeFlags( _x );
}

void CPU::DEY( u16 address )
{
    /*
     * @brief Decrement Y Register by One
     * N Z C I D V
     * + + - - - -
     * Usage and cycles:
     * DEY: 88(2)
     */
    (void) address;
    _y--;
    SetZeroAndNegativeFlags( _y );
}

void CPU::CLC( const u16 address )
{
    /* @brief Clear Carry Flag
     * N Z C I D V
     * - - 0 - - -
     *   Usage and cycles:
     *   CLC: 18(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Carry );
}

void CPU::CLI( const u16 address )
{
    /* @brief Clear Interrupt Disable
     * N Z C I D V
     * - - - 0 - -
     *   Usage and cycles:
     *   CLI: 58(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::InterruptDisable );
}
void CPU::CLD( const u16 address )
{
    /* @brief Clear Decimal Mode
     * N Z C I D V
     * - - - - 0 -
     *   Usage and cycles:
     *   CLD: D8(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Decimal );
}
void CPU::CLV( const u16 address )
{
    /* @brief Clear Overflow Flag
     * N Z C I D V
     * - - - - - 0
     *   Usage and cycles:
     *   CLV: B8(2)
     */
    (void) address;
    CPU::ClearFlags( CPU::Overflow );
}

void CPU::SEC( const u16 address )
{
    /* @brief Set Carry Flag
     * N Z C I D V
     * - - 1 - - -
     *   Usage and cycles:
     *   SEC: 38(2)
     */
    (void) address;
    CPU::SetFlags( CPU::Carry );
}

void CPU::SED( const u16 address )
{
    /* @brief Set Decimal Flag
     * N Z C I D V
     * - - - - 1 -
     *   Usage and cycles:
     *   SED: F8(2)
     */
    (void) address;
    CPU::SetFlags( CPU::Decimal );
}

void CPU::SEI( const u16 address )
{
    /* @brief Set Interrupt Disable
     * N Z C I D V
     * - - - 1 - -
     *   Usage and cycles:
     *   SEI: 78(2)
     */
    (void) address;
    CPU::SetFlags( CPU::InterruptDisable );
}

void CPU::BPL( const u16 address )
{
    /* @brief Branch if Positive
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BPL: 10(2+)
     */
    BranchOnStatus( address, Status::Negative, false );
}

void CPU::BMI( const u16 address )
{
    /* @brief Branch if Minus
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BMI: 30(2+)
     */
    BranchOnStatus( address, Status::Negative, true );
}

void CPU::BVC( const u16 address )
{
    /* @brief Branch if Overflow Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVC: 50(2+)
     */
    BranchOnStatus( address, Status::Overflow, false );
}

void CPU::BVS( const u16 address )
{
    /* @brief Branch if Overflow Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BVS: 70(2+)
     */
    BranchOnStatus( address, Status::Overflow, true );
}

void CPU::BCC( const u16 address )
{
    /* @brief Branch if Carry Clear
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCC: 90(2+)
     */
    BranchOnStatus( address, Status::Carry, false );
}

void CPU::BCS( const u16 address )
{
    /* @brief Branch if Carry Set
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BCS: B0(2+)
     */
    BranchOnStatus( address, Status::Carry, true );
}

void CPU::BNE( const u16 address )
{
    /* @brief Branch if Not Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BNE: D0(2+)
     */
    BranchOnStatus( address, Status::Zero, false );
}

void CPU::BEQ( const u16 address )
{
    /* @brief Branch if Equal
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   BEQ: F0(2+)
     */
    BranchOnStatus( address, Status::Zero, true );
}

void CPU::CMP( u16 address )
{
    /* @brief Compare Memory and Accumulator
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CMP Immediate: C9(2)
     *   CMP Zero Page: C5(3)
     *   CMP Zero Page X: D5(4)
     *   CMP Absolute: CD(4)
     *   CMP Absolute X: DD(4+)
     *   CMP Absolute Y: D9(4+)
     *   CMP Indirect X: C1(6)
     *   CMP Indirect Y: D1(5+)
     */
    CompareAddressWithRegister( address, _a );
}

void CPU::CPX( u16 address )
{
    /* @brief Compare Memory and X Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPX Immediate: E0(2)
     *   CPX Zero Page: E4(3)
     *   CPX Absolute: EC(4)
     */
    CompareAddressWithRegister( address, _x );
}

void CPU::CPY( u16 address )
{
    /* @brief Compare Memory and Y Register
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   CPY Immediate: C0(2)
     *   CPY Zero Page: C4(3)
     *   CPY Absolute: CC(4)
     */
    CompareAddressWithRegister( address, _y );
}

void CPU::PHA( const u16 address )
{
    /* @brief Push Accumulator on Stack
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   PHA: 48(3)
     */
    (void) address;

    // Get the stack pointer
    const u8 stack_pointer = GetStackPointer();

    // Push the accumulator onto the stack
    WriteAndTick( 0x0100 + stack_pointer, GetAccumulator() );

    // Decrement the stack pointer
    SetStackPointer( stack_pointer - 1 );
}

void CPU::PHP( const u16 address )
{
    /* @brief Push Processor Status on Stack
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   PHP: 08(3)
     */
    (void) address;

    // Get the stack pointer
    const u8 stack_pointer = GetStackPointer();

    // Set the Break flag before pushing the status register onto the stack
    u8 status = GetStatusRegister();
    status |= Break;

    // Push the modified status register onto the stack
    WriteAndTick( 0x0100 + stack_pointer, status );

    SetStackPointer( stack_pointer - 1 );
}

void CPU::PLA( const u16 address )
{
    /* @brief Pop Accumulator from Stack
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   PLA: 68(4)
     */
    (void) address;

    // Increment the stack pointer first
    SetStackPointer( GetStackPointer() + 1 );

    // Get the accumulator from the stack and set the zero and negative flags
    SetAccumulator( ReadAndTick( 0x100 + GetStackPointer() ) );
    Tick(); // Dummy read
    SetZeroAndNegativeFlags( _a );
}

void CPU::PLP( const u16 address )
{
    /* @brief Pop Processor Status from Stack
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   PLP: 28(4)
     */
    (void) address;

    // Increment the stack pointer first
    SetStackPointer( GetStackPointer() + 1 );

    SetStatusRegister( ReadAndTick( 0x100 + GetStackPointer() ) );
    ClearFlags( Status::Break );
    Tick(); // Dummy read
    SetFlags( Status::Unused );
}

void CPU::TSX( const u16 address )
{
    /* @brief Transfer Stack Pointer to X
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TSX: BA(2)
     */
    (void) address;
    SetXRegister( GetStackPointer() );
    SetZeroAndNegativeFlags( GetXRegister() );
}

void CPU::TXS( const u16 address )
{
    /* @brief Transfer X to Stack Pointer
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   TXS: 9A(2)
     */
    (void) address;
    SetStackPointer( GetXRegister() );
}

void CPU::ASL( u16 address )
{
    /* @brief Arithmetic Shift Left
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ASL Accumulator: 0A(2)
     *   ASL Zero Page: 06(5)
     *   ASL Zero Page X: 16(6)
     *   ASL Absolute: 0E(6)
     *   ASL Absolute X: 1E(7)
     */

    if ( _addr_mode == "IMP" )
    {
        u8 accumulator = GetAccumulator();
        // Set the carry flag if bit 7 is set
        ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // Shift the accumulator left by one
        accumulator <<= 1;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( accumulator );

        // Set the new accumulator value
        SetAccumulator( accumulator );
    }
    else
    {
        u8 const value = ReadAndTick( address );

        Tick(); // simulate dummy write

        // Set the carry flag if bit 7 is set
        ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        u8 const result = value << 1;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( result );

        // Write the result back to memory
        WriteAndTick( address, result );
    }
}

void CPU::LSR( u16 address )
{
    /* @brief Logical Shift Right
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   LSR Accumulator: 4A(2)
     *   LSR Zero Page: 46(5)
     *   LSR Zero Page X: 56(6)
     *   LSR Absolute: 4E(6)
     *   LSR Absolute X: 5E(7)
     */

    if ( _addr_mode == "IMP" )
    {
        u8 accumulator = GetAccumulator();
        // Set the carry flag if bit 0 is set
        ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // Shift the accumulator right by one
        accumulator >>= 1;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( accumulator );

        // Set the new accumulator value
        SetAccumulator( accumulator );
    }
    else
    {
        u8 const value = ReadAndTick( address );
        Tick(); // simulate dummy write

        // Set the carry flag if bit 0 is set
        ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        u8 const result = value >> 1;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( result );

        // Write the result back to memory
        WriteAndTick( address, result );
    }
}

void CPU::ROL( u16 address )
{
    /* @brief Rotate Left
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ROL Accumulator: 2A(2)
     *   ROL Zero Page: 26(5)
     *   ROL Zero Page X: 36(6)
     *   ROL Absolute: 2E(6)
     *   ROL Absolute X: 3E(7)
     */

    const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;
    if ( _addr_mode == "IMP" )
    {
        u8 accumulator = GetAccumulator();

        // Set the carry flag if bit 7 is set
        ( accumulator & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // Shift the accumulator left by one
        accumulator <<= 1;

        // Add the carry to the accumulator
        accumulator |= carry;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( accumulator );

        // Set the new accumulator value
        SetAccumulator( accumulator );
    }
    else
    {
        u8 const value = ReadAndTick( address );
        Tick(); // dummy write

        // Set the carry flag if bit 7 is set
        ( value & 0b10000000 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        u8 result = value << 1;
        result |= carry;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( result );

        // Write the result back to memory
        WriteAndTick( address, result );
    }
}

void CPU::ROR( u16 address )
{
    /* @brief Rotate Right
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ROR Accumulator: 6A(2)
     *   ROR Zero Page: 66(5)
     *   ROR Zero Page X: 76(6)
     *   ROR Absolute: 6E(6)
     *   ROR Absolute X: 7E(7)
     */

    const u8 carry = IsFlagSet( Status::Carry ) ? 1 : 0;

    if ( _addr_mode == "IMP" )
    { // implied mode
        u8 accumulator = GetAccumulator();

        // Set the carry flag if bit 0 is set
        ( accumulator & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        // Shift the accumulator right by one
        accumulator >>= 1;

        // Add the carry to the accumulator
        accumulator |= carry << 7;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( accumulator );

        // Set the new accumulator value
        SetAccumulator( accumulator );
    }
    else
    { // Memory mode
        u8 const value = ReadAndTick( address );
        Tick(); // simulate dummy write

        // Set the carry flag if bit 0 is set
        ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

        u8 result = value >> 1;
        result |= carry << 7;

        // Set the zero and negative flags
        SetZeroAndNegativeFlags( result );

        // Write the result back to memory
        WriteAndTick( address, result );
    }
}

void CPU::JMP( u16 address )
{
    /* @brief Jump to New Location
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   JMP Absolute: 4C(3)
     *   JMP Indirect: 6C(5)
     */
    _pc = address;
}

void CPU::JSR( u16 address )
{
    /* @brief Jump to Sub Routine, Saving Return Address
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   JSR Absolute: 20(6)
     */
    u16 const return_address = _pc - 1;
    Tick(); // Additional read here, probably for timing purposes
    StackPush( ( return_address >> 8 ) & 0xFF );
    StackPush( return_address & 0xFF );
    _pc = address;
}

void CPU::RTS( const u16 address )
{
    /* @brief Return from Subroutine
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   RTS: 60(6)
     */
    (void) address;
    u16 const low = StackPop();
    u16 const high = StackPop();
    _pc = ( high << 8 ) | low;
    Tick(); // Account for reading the new address
    _pc++;
    Tick(); // Account for reading the next pc value
}

void CPU::RTI( const u16 address )
{
    /* @brief Return from Interrupt
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   RTI: 40(6)
     */
    (void) address;
    u8 const status = StackPop();

    // Ignore the break flag and ensure the unused flag (bit 5) is set
    _p = ( status & ~Break ) | Unused;

    u16 const low = StackPop();
    u16 const high = StackPop();
    _pc = ( high << 8 ) | low;
    Tick(); // Account for reading the new address
}

void CPU::BRK( const u16 address )
{
    /* @brief Force Interrupt
     * N Z C I D V
     * from stack
     *   Usage and cycles:
     *   BRK: 00(7)
     * Cycles:
     *   Read opcode: 1, Read padding byte: 1
     *   Push PC(2): 2, Push status(1): 1
     *   Read vector low: 1, Read vector high: 1
     */
    (void) address;
    _pc++; // padding byte

    // Push pc to the stack
    StackPush( _pc >> 8 );     // 1 cycle
    StackPush( _pc & 0x00FF ); // 1 cycle

    // Push status with break and unused flag set (ignored when popped)
    StackPush( _p | Break | Unused );

    // Set PC to the value at the interrupt vector (0xFFFE)
    u16 const low = ReadAndTick( 0xFFFE );
    u16 const high = ReadAndTick( 0xFFFF );
    _pc = ( high << 8 ) | low;

    // Set the interrupt disable flag
    SetFlags( InterruptDisable );
}

void CPU::AND( u16 address )
{
    /* @brief XOR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   AND Immediate: 29(2)
     *   AND Zero Page: 25(3)
     *   AND Zero Page X: 35(4)
     *   AND Absolute: 2D(4)
     *   AND Absolute X: 3D(4+)
     *   AND Absolute Y: 39(4+)
     *   AND Indirect X: 21(6)
     *   AND Indirect Y: 31(5+)
     */
    u8 const value = ReadAndTick( address );
    _a &= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::ORA( const u16 address )
{
    /* @brief OR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   ORA Immediate: 09(2)
     *   ORA Zero Page: 05(3)
     *   ORA Zero Page X: 15(4)
     *   ORA Absolute: 0D(4)
     *   ORA Absolute X: 1D(4+)
     *   ORA Absolute Y: 19(4+)
     *   ORA Indirect X: 01(6)
     *   ORA Indirect Y: 11(5+)
     */

    u8 const value = ReadAndTick( address ); // 1 cycle
    _a |= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::EOR( const u16 address )
{
    /* @brief XOR Memory with Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   EOR Immediate: 49(2)
     *   EOR Zero Page: 45(3)
     *   EOR Zero Page X: 55(4)
     *   EOR Absolute: 4D(4)
     *   EOR Absolute X: 5D(4+)
     *   EOR Absolute Y: 59(4+)
     *   EOR Indirect X: 41(6)
     *   EOR Indirect Y: 51(5+)
     */
    u8 const value = ReadAndTick( address );
    _a ^= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::BIT( const u16 address )
{
    /* @brief Test Bits in Memory with Accumulator
     * Performs AND between accumulator and memory, but does not store the result
     * N Z C I D V
     * + + - - - +
     *   Usage and cycles:
     *   BIT Zero Page: 24(3)
     *   BIT Absolute: 2C(4)
     */

    u8 const value = ReadAndTick( address );
    SetZeroAndNegativeFlags( _a & value );

    // Set overflow flag to bit 6 of value
    ( value & 0b01000000 ) != 0 ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );

    // Set negative flag to bit 7 of value
    ( value & 0b10000000 ) != 0 ? SetFlags( Status::Negative ) : ClearFlags( Status::Negative );
}

void CPU::TAX( const u16 address )
{
    /* @brief Transfer Accumulator to X Register
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TAX: AA(2)
     */
    (void) address;
    SetXRegister( GetAccumulator() );
    SetZeroAndNegativeFlags( GetXRegister() );
}

void CPU::TXA( const u16 address )
{
    /* @brief Transfer X Register to Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TXA: 8A(2)
     */
    (void) address;
    SetAccumulator( GetXRegister() );
    SetZeroAndNegativeFlags( GetAccumulator() );
}

void CPU::TAY( const u16 address )
{
    /* @brief Transfer Accumulator to Y Register
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TAY: A8(2)
     */
    (void) address;
    SetYRegister( GetAccumulator() );
    SetZeroAndNegativeFlags( GetYRegister() );
}

void CPU::TYA( const u16 address )
{
    /* @brief Transfer Y Register to Accumulator
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   TYA: 98(2)
     */
    (void) address;
    SetAccumulator( GetYRegister() );
    SetZeroAndNegativeFlags( GetAccumulator() );
}

/*
################################################################
||                                                            ||
||                      Illegal Opcodes                       ||
||                                                            ||
################################################################
*/

void CPU::NOP2( u16 address ) // NOLINT
{
    /*
     * @brief No operation, has an additional cycle
     * N Z C I D V
     * - - - - - -
     * --  Illegal  --
     * NOP Immediate: 80(2)
     * NOP Immediate: 82(2)
     * NOP Immediate: 89(2)
     * NOP Immediate: C2(2)
     * NOP Immediate: E2(2)
     * NOP Zero Page: 04(3)
     * NOP Zero Page: 44(3)
     * NOP Zero Page: 64(3)
     * NOP Zero Page X: 14(4)
     * NOP Zero Page X: 34(4)
     * NOP Zero Page X: 54(4)
     * NOP Zero Page X: 74(4)
     * NOP Zero Page X: D4(4)
     * NOP Zero Page X: F4(4)
     * NOP Absolute: 0C(4)
     * NOP Absolute: 1C(4)
     * NOP Absolute: 3C(4)
     * NOP Absolute: 5C(4)
     * NOP Absolute: 7C(4)
     * NOP Absolute: DC(4)
     * NOP Absolute: FC(4)
     */
    Tick();
    (void) address;
}
void CPU::JAM( const u16 address ) // NOLINT
{
    /* @brief Illegal Opcode
     * Freezes the hardware, usually never called
     * Tom Harte tests include these, though, so for completeness, we'll add them
     */
    (void) address;
    for ( int i = 0; i < 9; i++ )
    {
        Tick();
    }
}

void CPU::SLO( const u16 address )
{
    /* @brief Illegal opcode: combines ASL and ORA
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   SLO Zero Page: 07(5)
     *   SLO Zero Page X: 17(6)
     *   SLO Absolute: 0F(6)
     *   SLO Absolute X: 1F(7)
     *   SLO Absolute Y: 1B(7)
     *   SLO Indirect X: 03(8)
     *   SLO Indirect Y: 13(8)
     */
    CPU::ASL( address );

    // ORA is side effect, no cycles are spent
    u8 const value = Read( address ); // 0 cycle
    _a |= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::SAX( const u16 address ) // NOLINT
{
    /* @brief Illegal opcode: combines STX and AND
     * N Z C I D V
     * - - - - - -
     *   Usage and cycles:
     *   SAX Zero Page: 87(3)
     *   SAX Zero Page Y: 97(4)
     *   SAX Indirect X: 83(6)
     *   SAX Absolute: 8F(4)
     */
    WriteAndTick( address, _a & _x );
}

void CPU::LXA( const u16 address )
{
    /* @brief Illegal opcode: combines LDA and LDX
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   LXA Immediate: AB(2)
     */

    u8 const magic_constant = 0xEE;
    u8 const value = ReadAndTick( address );

    u8 const result = ( ( _a | magic_constant ) & value );
    _a = result;
    _x = result;
    SetZeroAndNegativeFlags( _a );
}

void CPU::LAX( const u16 address )
{
    /* @brief Illegal opcode: combines LDA and LDX
     * N Z C I D V
     * + + - - - -
     *   Usage and cycles:
     *   LAX Zero Page: A7(3)
     *   LAX Zero Page Y: B7(4)
     *   LAX Absolute: AF(4)
     *   LAX Absolute Y: BF(4+)
     *   LAX Indirect X: A3(6)
     *   LAX Indirect Y: B3(5+)
     */
    u8 const value = ReadAndTick( address );
    SetAccumulator( value );
    SetXRegister( value );
    SetZeroAndNegativeFlags( value );
}

void CPU::ARR( const u16 address )
{
    /* @brief Illegal opcode: combines AND and ROR
     * N Z C I D V
     * + + + - - +
     *   Usage and cycles:
     *   ARR Immediate: 6B(2)
     */

    // A & operand
    u8 value = _a & ReadAndTick( address );

    // ROR
    u8 const carry_in = IsFlagSet( Status::Carry ) ? 0x80 : 0x00;
    value = ( value >> 1 ) | carry_in;

    _a = value;

    // Set flags
    SetZeroAndNegativeFlags( _a );

    // Adjust C and V flags according to the ARR rules
    // C = bit 6 of A
    ( _a & 0x40 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );

    // V = bit 5 XOR bit 6
    bool const is_overflow = ( ( _a & 0x40 ) != 0 ) ^ ( ( _a & 0x20 ) != 0 );
    ( is_overflow ) ? SetFlags( Status::Overflow ) : ClearFlags( Status::Overflow );
}

void CPU::ALR( const u16 address )
{
    /* @brief Illegal opcode: combines AND and LSR
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ALR Immediate: 4B(2)
     */
    CPU::AND( address );

    u8 const value = GetAccumulator();
    ( value & 0b00000001 ) != 0 ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    u8 const result = value >> 1;
    SetZeroAndNegativeFlags( result );
    _a = result;
}

void CPU::RRA( const u16 address )
{
    /* @brief Illegal opcode: combines ROR and ADC
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   RRA Zero Page: 67(5)
     *   RRA Zero Page X: 77(6)
     *   RRA Absolute: 6F(6)
     *   RRA Absolute X: 7F(7)
     *   RRA Absolute Y: 7B(7)
     *   RRA Indirect X: 63(8)
     *   RRA Indirect Y: 73(8)
     */
    CPU::ROR( address );
    CPU::ADC( address );
}

void CPU::SRE( const u16 address )
{
    /* @brief Illegal opcode: combines LSR and EOR
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   SRE Zero Page: 47(5)
     *   SRE Zero Page X: 57(6)
     *   SRE Absolute: 4F(6)
     *   SRE Absolute X: 5F(7)
     *   SRE Absolute Y: 5B(7)
     *   SRE Indirect X: 43(8)
     *   SRE Indirect Y: 53(8)
     */
    CPU::LSR( address );

    // Free side effect
    u8 const value = Read( address ); // 0 cycle
    _a ^= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::RLA( const u16 address )
{
    /* @brief Illegal opcode: combines ROL and AND
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   RLA Zero Page: 27(5)
     *   RLA Zero Page X: 37(6)
     *   RLA Absolute: 2F(6)
     *   RLA Absolute X: 3F(7)
     *   RLA Absolute Y: 3B(7)
     *   RLA Indirect X: 23(8)
     *   RLA Indirect Y: 33(8)
     */
    CPU::ROL( address );

    // Free side effect
    u8 const value = Read( address ); // 0 cycle
    _a &= value;
    SetZeroAndNegativeFlags( _a );
}

void CPU::DCP( const u16 address )
{
    /* @brief Illegal opcode: combines DEC and CMP
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   DCP Zero Page: C7(5)
     *   DCP Zero Page X: D7(6)
     *   DCP Absolute: CF(6)
     *   DCP Absolute X: DF(7)
     *   DCP Absolute Y: DB(7)
     *   DCP Indirect X: C3(8)
     *   DCP Indirect Y: D3(8)
     */
    CPU::DEC( address );
    CPU::CMP( address );
}

void CPU::ISC( const u16 address )
{
    /* @brief Illegal opcode: combines INC and SBC
     * N Z C I D V
     * + + + - - +
     *   Usage and cycles:
     *   ISC Zero Page: E7(5)
     *   ISC Zero Page X: F7(6)
     *   ISC Absolute: EF(6)
     *   ISC Absolute X: FF(7)
     *   ISC Absolute Y: FB(7)
     *   ISC Indirect X: E3(8)
     *   ISC Indirect Y: F3(8)
     */
    CPU::INC( address );
    CPU::SBC( address );
}

void CPU::ANC( const u16 address )
{
    /* @brief Illegal opcode: combines AND and Carry
     * N Z C I D V
     * + + + - - -
     *   Usage and cycles:
     *   ANC Immediate: 0B(2)
     *   ANC Immediate: 2B(2)
     */
    CPU::AND( address );
    ( IsFlagSet( Status::Negative ) ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
}

void CPU::SBX( const u16 address )
{
    /* @brief Illegal opcode: SBX (a.k.a. AXS) combines CMP and DEX
     *        (A & X) - immediate -> X
     * Sets flags like CMP:
     *   N Z C I D V
     *   + + + - - -
     *
     * Usage and cycles:
     *   SBX Immediate: CB (2 bytes, 2 cycles)
     */
    u8 const  operand = ReadAndTick( address );
    u8 const  left = ( _a & _x );
    u16 const diff = static_cast<uint16_t>( left ) - static_cast<uint16_t>( operand );
    _x = static_cast<u8>( diff & 0xFF );
    ( ( diff & 0x100 ) == 0 ) ? SetFlags( Status::Carry ) : ClearFlags( Status::Carry );
    SetZeroAndNegativeFlags( _x );
}
