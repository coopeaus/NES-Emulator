#include "bus.h"
#include <fmt/base.h>
#include <pybind11/pybind11.h>
#include "paths.h"

namespace py = pybind11;

class Emulator
{
  public:
    Bus        bus;
    PPU       &ppu = bus.ppu;
    CPU       &cpu = bus.cpu;
    Cartridge &cart = bus.cartridge;

    Emulator() = default;

    /*
    #######################################
    ||            CPU Getters            ||
    #######################################
    */
    u64 GetCycles() const { return bus.cpu.GetCycles(); }
    u8  A() const { return cpu.GetAccumulator(); }
    u8  X() const { return cpu.GetXRegister(); }
    u8  Y() const { return cpu.GetYRegister(); }
    u8  SP() const { return cpu.GetStackPointer(); }
    u16 PC() const { return cpu.GetProgramCounter(); }
    u8  P() const { return cpu.GetStatusRegister(); }

    u8 CarryFlag() const { return cpu.GetCarryFlag(); }
    u8 ZeroFlag() const { return cpu.GetZeroFlag(); }
    u8 InterruptFlag() const { return cpu.GetInterruptDisableFlag(); }
    u8 DecimalFlag() const { return cpu.GetDecimalFlag(); }
    u8 BreakFlag() const { return cpu.GetBreakFlag(); }
    u8 OverflowFlag() const { return cpu.GetOverflowFlag(); }
    u8 NegativeFlag() const { return cpu.GetNegativeFlag(); }

    /*
    #######################################
    ||            CPU Setters            ||
    #######################################
    */
    void SetCycles( u64 value ) { cpu.SetCycles( value ); }
    void SetA( u8 value ) { cpu.SetAccumulator( value ); }
    void SetX( u8 value ) { cpu.SetXRegister( value ); }
    void SetY( u8 value ) { cpu.SetYRegister( value ); }
    void SetSP( u8 value ) { cpu.SetStackPointer( value ); }
    void SetPC( u16 value ) { cpu.SetProgramCounter( value ); }
    void SetP( u8 value ) { cpu.SetStatusRegister( value ); }

    /*
    #######################################
    ||            PPU Getters            ||
    #######################################
    */
    u8  GetNmi() const { return ppu.GetCtrlNmiEnable(); }
    u8  GetVblank() const { return ppu.GetStatusVblank(); }
    u16 GetScanline() const { return ppu.scanline; }
    u16 GetPpuCycles() const { return ppu.cycle; }
    u64 GetFrame() const { return ppu.frame; }

    /*
    ################################
    ||         PPU Setters        ||
    ################################
    */
    void SetScanline( s16 value ) { ppu.scanline = value; }
    void SetPpuCycles( s16 value ) { ppu.SetCycles( value ); }

    /*
    #######################################
    ||         Cartridge Getters         ||
    #######################################
    */
    bool DidMapperLoad() const { return cart.DidMapperLoad(); }
    bool DoesMapperExist() const { return cart.DoesMapperExist(); }

    /*
    ################################
    ||     Cartrdidge Setters     ||
    ################################
    */

    /*
    #######################################
    ||              Methods              ||
    #######################################
    */
    void Load( const std::string &path ) { bus.cartridge.LoadRom( path ); }

    void Preset()
    {
        // Loads a common rom used for debugging to get going quickly.
        std::string romFile = std::string( paths::roms() ) + "/custom.nes";
        bus.cartridge.LoadRom( romFile );
        bus.cpu.Reset();
    }

    void DebugReset() { bus.DebugReset(); }

    static void Test() { fmt::print( "Test\n" ); }
    void        Log()
    {
        auto out = bus.cpu.LogLineAtPC();
        fmt::print( "{}\n", out );
    }

    void Step( int n = 1 )
    {
        for ( int i = 0; i < n; i++ ) {
            bus.Clock();
        }
    }

    u8 Read( u16 addr ) const { return bus.cpu.Read( addr ); }
    u8 PpuRead( u16 addr ) { return bus.ppu.ReadVram( addr ); }

    void EnableMesenTrace( int n = 100 )
    {
        cpu.EnableMesenFormatTraceLog();
        cpu.SetMesenTraceSize( n );
    }
    void DisableMesenTrace() { cpu.DisableMesenFormatTraceLog(); }
    void PrintMesenTrace() const
    {
        for ( const auto &line : cpu.GetMesenFormatTracelog() ) {
            fmt::print( "{}", line );
        }
    }
};

PYBIND11_MODULE( emu, m ) // <-- Python module name. Must match the name in the CMakeLists
{
    py::class_<Emulator>( m, "Emulator" )
        .def( py::init<>() )
        // CPU Getters
        .def_property_readonly( "cpu_cycles", &Emulator::GetCycles, "Get the number of cycles" )
        .def_property_readonly( "a", &Emulator::A, "Get the accumulator" )
        .def_property_readonly( "x", &Emulator::X, "Get the X register" )
        .def_property_readonly( "y", &Emulator::Y, "Get the Y register" )
        .def_property_readonly( "sp", &Emulator::SP, "Get the stack pointer" )
        .def_property_readonly( "pc", &Emulator::PC, "Get the program counter" )
        .def_property_readonly( "p", &Emulator::P, "Get the status register" )
        .def_property_readonly( "carry_flag", &Emulator::CarryFlag, "Get the carry flag" )
        .def_property_readonly( "zero_flag", &Emulator::ZeroFlag, "Get the zero flag" )
        .def_property_readonly( "interrupt_flag", &Emulator::InterruptFlag, "Get the interrupt flag" )
        .def_property_readonly( "decimal_flag", &Emulator::DecimalFlag, "Get the decimal flag" )
        .def_property_readonly( "break_flag", &Emulator::BreakFlag, "Get the break flag" )
        .def_property_readonly( "overflow_flag", &Emulator::OverflowFlag, "Get the overflow flag" )
        .def_property_readonly( "negative_flag", &Emulator::NegativeFlag, "Get the negative flag" )
        // CPU Setters
        .def( "set_cycles", &Emulator::SetCycles, "Set the number of cycles" )
        .def( "set_a", &Emulator::SetA, "Set the accumulator" )
        .def( "set_x", &Emulator::SetX, "Set the X register" )
        .def( "set_y", &Emulator::SetY, "Set the Y register" )
        .def( "set_sp", &Emulator::SetSP, "Set the stack pointer" )
        .def( "set_pc", &Emulator::SetPC, "Set the program counter" )
        .def( "set_p", &Emulator::SetP, "Set the status register" )
        // PPU Getters
        .def_property_readonly( "nmi", &Emulator::GetNmi, "Get the PPU NMI flag" )
        .def_property_readonly( "vblank", &Emulator::GetVblank, "Get the PPU VBLANK flag" )
        .def_property_readonly( "scanline", &Emulator::GetScanline, "Get the scanline" )
        .def_property_readonly( "ppu_cycles", &Emulator::GetPpuCycles, "Get the PPU cycles" )
        .def_property_readonly( "frame", &Emulator::GetFrame, "Get the frame" )
        // PPU Setters
        .def( "set_scanline", &Emulator::SetScanline, "Set the scanline" )
        .def( "set_ppu_cycles", &Emulator::SetPpuCycles, "Set the PPU cycles" )
        // Cartridge Getters
        .def_property_readonly( "did_mapper_load", &Emulator::DidMapperLoad, "Get if the mapper loaded" )
        .def_property_readonly( "does_mapper_exist", &Emulator::DoesMapperExist, "Get if the mapper exists" )
        // Cartridge Setters
        // Methods
        .def( "load", &Emulator::Load, "Load a rom file" )
        .def( "preset", &Emulator::Preset, "Load custom.nes rom for debugging" )
        .def( "debug_reset", &Emulator::DebugReset, "Reset the CPU and PPU" )
        .def( "log", &Emulator::Log, "Log CPU state" )
        .def( "step", &Emulator::Step, "Step the CPU by one or more cycles", py::arg( "n" ) = 1 )
        .def( "enable_mesen_trace", &Emulator::EnableMesenTrace, "Enable Mesen trace log",
              py::arg( "n" ) = 100 )
        .def( "disable_mesen_trace", &Emulator::DisableMesenTrace, "Disable Mesen trace log" )
        .def( "print_mesen_trace", &Emulator::PrintMesenTrace, "Print Mesen trace log" )
        .def( "read", &Emulator::Read, "Read from CPU memory", py::arg( "addr" ) )
        .def( "ppu_read", &Emulator::PpuRead, "Read from PPU memory", py::arg( "addr" ) )
        .def_static( "test", &Emulator::Test, "Test function" );
}
