#pragma once
#include "bus.h"
#include "ines2.h"
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class CartridgeInfoWindow : public UIComponent
{
  public:
    CartridgeInfoWindow( Renderer *renderer )
        : UIComponent( renderer ), cpu( renderer->bus.cpu ), iNes( renderer->bus.cartridge.iNes ),
          byte4( iNes.header.fields.prgRomSizeLSB ), byte5( iNes.header.fields.chrRomSizeLSB ),
          byte6( iNes.header.fields.flag6.value ), byte7( iNes.header.fields.flag7.value ),
          byte8( iNes.header.fields.mapperMSB.value ), byte9( iNes.header.fields.romSizeMSB.value ),
          byte10( iNes.header.fields.chrRamSize.value ), byte11( iNes.header.fields.chrRamSize.value ),
          byte12( iNes.header.fields.cpuPpuTiming.value ), byte13( iNes.header.fields.vsSystemType.value ),
          byte14( iNes.header.fields.miscRoms.value ),
          byte15( iNes.header.fields.defaultExpansionDevice.value )
    {
        visible = false;
    }

    CPU           &cpu;  // NOLINT
    iNes2Instance &iNes; // NOLINT

    u8 &byte4;
    u8 &byte5;
    u8 &byte6;
    u8 &byte7;
    u8 &byte8;
    u8 &byte9;
    u8 &byte10;
    u8 &byte11;
    u8 &byte12;
    u8 &byte13;
    u8 &byte14;
    u8 &byte15;

    /*
    ################################
    #           Variables          #
    ################################
    */

    /*
    ################################
    #            Methods           #
    ################################
    */
    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override
    {
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |
                                                 ImGuiWindowFlags_NoScrollbar |
                                                 ImGuiWindowFlags_NoScrollWithMouse;
        ImVec2 const windowSize = ImVec2( 700, 550 );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( windowSize, windowSize );

        ImGui::Begin( "Cartridge Info", &visible, windowFlags );
        RenderMenuBar();
        ImGui::Spacing();
        InfoBox( ImVec2( 0, 60 ) );
        ImGui::PushFont( renderer->fontMono );
        HeaderInfoRaw( ImVec2( 280, 0 ) );
        ImGui::SameLine();
        CartridgeInfo( ImVec2( 380, 0 ) );
        ImGui::PopFont();

        ImGui::Spacing();
        ImGui::End();
        ImGui::PopStyleVar();
    }

    static void InfoBox( ImVec2 size )
    {
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ImGui::BeginChild( "info box", size, ImGuiChildFlags_Borders );
        ImGui::Text(
            "ROM Header Information"
            "\nClick individual bits to see how derived information is affected (emulation not effected)." );

        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void HeaderInfoRaw( ImVec2 size )
    {

        ImGui::BeginChild( "header info wrapper", ImVec2( size.x, 0 ) );
        ImGui::SeparatorText( "iNes Header" );
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ImGui::BeginChild( "header info raw", size, ImGuiChildFlags_Borders );
        ImGui::PopStyleColor();

        float const innerSpacing = 80.0f;
        float const outerSpacing = 90.0f;

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 0-3" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        std::string id = iNes.GetBytes0to3();
        if ( id.size() == 4 && id[3] == '\x1A' ) {
            id = id.substr( 0, 3 ) + "<EOF>";
        }
        ImGui::Text( "%s", id.c_str() );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( "Identification String. Must be NES<EOF>." );
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 4" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 4, &byte4 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( "PRG-ROM size LSB" );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 5" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 5, &byte5 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( "CHR-ROM size LSB" );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 6" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 6, &byte6 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( "Flags 6\nD~7654 3210\n---------\n NNNN FTBM\n |||| |||+-- Hard-wired nametable layout\n "
                    "|||| |||     0: Vertical arrangement (mirrored horizontally) or mapper-controlled \n "
                    "|||| |||     1: Horizontal arrangement (mirrored vertically) \n |||| ||+--- Battery and "
                    "other non-volatile memory \n |||| ||      0: Not present \n |||| ||      1: Present\n "
                    "|||| |+--- 512-byte Trainer\n |||| |      0: Not present\n |||| |      1: Present "
                    "between Header and PRG-ROM data\n |||| +---- Alternative nametables\n ||||        0: "
                    "No\n ||||        1: Yes\n ++++------ Mapper Number D3..D0\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 7" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 7, &byte7 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " Flags 7\n D~7654 3210\n ---------\n NNNN 10TT\n |||| ||++- Console type\n |||| ||     "
                    "0: Nintendo Entertainment System/Family Computer\n |||| ||     1: Nintendo Vs. System\n "
                    "|||| ||     2: Nintendo Playchoice 10\n |||| ||     3: Extended Console Type\n |||| "
                    "++--- NES 2.0 identifier\n ++++------ Mapper Number D7..D4\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 8" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 8, &byte8 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " Mapper MSB/Submapper\n D~7654 3210\n ---------\n SSSS NNNN\n |||| ++++- Mapper number "
                    "D11..D8\n ++++------ Submapper number\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 9" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 9, &byte9 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " PRG-ROM/CHR-ROM size MSB\n D~7654 3210\n ---------\n CCCC PPPP\n |||| ++++- PRG-ROM "
                    "size MSB\n ++++------ CHR-ROM size MSB " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 10" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 10, &byte10 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker(
            " PRG-RAM/EEPROM size\n D~7654 3210\n ---------\n pppp PPPP\n |||| ++++- PRG-RAM (volatile) "
            "shift count\n ++++------ PRG-NVRAM/EEPROM (non-volatile) shift count\n If the shift count is "
            "zero, there is no PRG-(NV)RAM.\n If the shift count is non-zero, the actual size is\n '64 << "
            "shift count' bytes, i.e. 8192 bytes for a shift count of 7.\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 11" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 11, &byte11 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker(
            " CHR-RAM size\n D~7654 3210\n ---------\n cccc CCCC\n |||| ++++- CHR-RAM size (volatile) shift "
            "count\n ++++------ CHR-NVRAM size (non-volatile) shift count\n If the shift count is zero, "
            "there is no CHR-(NV)RAM.\n If the shift count is non-zero, the actual size is\n '64 << shift "
            "count' bytes, i.e. 8192 bytes for a shift count of 7.\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 12" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 12, &byte12 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker(
            " CPU/PPU Timing\n D~7654 3210\n ---------\n .... ..VV\n ++- CPU/PPU timing mode\n 0: RP2C02 "
            "('NTSC NES')\n 1: RP2C07 ('Licensed PAL NES')\n 2: Multiple-region\n 3: UA6538 ('Dendy')\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 13" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 13, &byte13 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " When Byte 7 AND 3 =1: Vs. System Type\n D~7654 3210\n ---------\n MMMM PPPP\n |||| "
                    "++++- Vs. PPU Type\n ++++------ Vs. Hardware Type\n\n When Byte 7 AND 3 =3: Extended "
                    "Console Type\n D~7654 3210\n ---------\n .... CCCC\n ++++- Extended Console Type\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 14" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 14, &byte14 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " Miscellaneous ROMs\n D~7654 3210\n ---------\n .... ..RR\n ++- Number of miscellaneous "
                    "ROMs present\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Byte 15" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        BinaryButtons( 15, &byte15 );
        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        HelpMarker( " Default Expansion Device\n D~7654 3210\n ---------\n ..DD DDDD\n ++-++++- Default "
                    "Expansion Device\n " );
        ImGui::SameLine();
        ImGui::EndGroup();

        ImGui::EndChild();
        ImGui::EndChild();
    }

    void CartridgeInfo( ImVec2 size )
    {
        ImGui::BeginChild( "derived info wrapper", ImVec2( size.x, 0 ) );
        ImGui::SeparatorText( "Drived Info" );
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1, 1, 1, 1 ) );
        ImGui::BeginChild( "header info derived", size, ImGuiChildFlags_Borders );
        ImGui::PopStyleColor();

        float const innerSpacing = 150.0f;
        float const outerSpacing = 180.0f;

        // Header Format (NES 2.0 vs. iNES)
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Header Version" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int nes20 = iNes.GetNes20Identifier();
            ImGui::Text( "%s", ( nes20 == 2 ) ? "NES 2.0" : "iNES" );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Flag7: Next 2 bits indicate NES 2.0 if equal to 2." );
            ImGui::EndGroup();
        }

        // PRG ROM Banks and Size
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "PRG ROM Banks:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int prgBanks = iNes.GetPrgRomBanks();
            int prgSize = iNes.GetPrgRomSizeBytes();
            ImGui::Text( "%d (%d KiB)", prgBanks, prgSize / 1024 );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Number of 16KB PRG ROM banks.\nByte 4: LSB, Byte 9: MSB." );
            ImGui::EndGroup();
        }

        // CHR ROM Banks and Size
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "CHR ROM Banks:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int chrBanks = iNes.GetChrRomBanks();
            int chrSize = iNes.GetChrRomSizeBytes();
            ImGui::Text( "%d (%d KiB)", chrBanks, chrSize / 1024 );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Number of 8KB CHR ROM banks.\nByte 5: LSB, Byte 9: MSB." );
            ImGui::EndGroup();
        }

        // Mapper Number
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Mapper:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int mapper = iNes.GetMapper();
            ImGui::Text( "%d", mapper );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Derived from Flag6, Flag7, and Mapper MSB." );
            ImGui::EndGroup();
        }

        // Nametable Mirroring
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Mirroring:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int mode = iNes.GetMirroring();
            ImGui::Text( "%s", mode == 0 ? "Horizontal" : "Vertical" );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Derived from Flag6 mirroring bit.\n0 = Vertical (mirrored horizontally), 1 = "
                        "Horizontal (mirrored vertically)." );
            ImGui::EndGroup();
        }

        // Battery-backed Memory
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Battery:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            ImGui::Text( "%s", iNes.GetBatteryMode() == 1 ? "Yes" : "No" );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Flag6 battery: 1 = present, 0 = not present." );
            ImGui::EndGroup();
        }

        // Trainer Presence
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Trainer:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            ImGui::Text( "%s", iNes.GetTrainerMode() == 1 ? "Yes" : "No" );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Flag6 trainer: 1 = present (512 bytes between header and PRG-ROM), 0 = absent." );
            ImGui::EndGroup();
        }

        // Four-Screen Layout
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Four Screen:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            ImGui::Text( "%s", iNes.GetFourScreenMode() == 1 ? "Yes" : "No" );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Flag6 fourScreen: 1 = four-screen layout, 0 = standard." );
            ImGui::EndGroup();
        }

        // Console Type
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Console Type:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int         consoleType = iNes.GetConsoleType();
            const char *consoleStr = "Unknown";
            switch ( consoleType ) {
                case 0:
                    consoleStr = "NES/Famicom";
                    break;
                case 1:
                    consoleStr = "Vs. System";
                    break;
                case 2:
                    consoleStr = "Playchoice 10";
                    break;
                case 3:
                    consoleStr = "Extended";
                    break;
                default:
                    break;
            }
            ImGui::Text( "%s", consoleStr );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Flag7: Console type (first 2 bits)." );
            ImGui::EndGroup();
        }

        // Submapper
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Submapper:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int submapper = iNes.GetSubmapper();
            ImGui::Text( "%d", submapper );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Mapper MSB (upper 4 bits of Byte 8) provides the submapper number." );
            ImGui::EndGroup();
        }

        // PRG-RAM and PRG-NVRAM Sizes
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "PRG RAM:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int prgRam = iNes.GetPrgRamSizeBytes();
            ImGui::Text( "%d bytes", prgRam );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Calculated from PRG-RAM shift count (Byte 10)." );
            ImGui::EndGroup();

            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "PRG NVRAM:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int prgNvram = iNes.GetPrgNvramSizeBytes();
            ImGui::Text( "%d bytes", prgNvram );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Calculated from PRG-NVRAM shift count (Byte 10)." );
            ImGui::EndGroup();
        }

        // CHR-RAM and CHR-NVRAM Sizes
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "CHR RAM:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int chrRam = iNes.GetChrRamSizeBytes();
            ImGui::Text( "%d bytes", chrRam );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Calculated from CHR-RAM shift count (Byte 11)." );
            ImGui::EndGroup();

            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "CHR NVRAM:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int chrNvram = iNes.GetChrNvramSizeBytes();
            ImGui::Text( "%d bytes", chrNvram );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Calculated from CHR-NVRAM shift count (Byte 11)." );
            ImGui::EndGroup();
        }

        // CPU/PPU Timing
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "CPU/PPU Timing:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int         timing = iNes.GetCpuPpuTiming();
            const char *timingStr = "Unknown";
            switch ( timing ) {
                case 0:
                    timingStr = "RP2C02 (NTSC)";
                    break;
                case 1:
                    timingStr = "RP2C07 (PAL)";
                    break;
                case 2:
                    timingStr = "Multiple-region";
                    break;
                case 3:
                    timingStr = "UA6538 (Dendy)";
                    break;
                default:
                    break;
            }
            ImGui::Text( "%s", timingStr );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "CPU/PPU timing mode (Byte 12)." );
            ImGui::EndGroup();
        }

        // Vs. System Info
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Vs. System:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int vsPpu = iNes.GetVsPpuType();
            int vsHardware = iNes.GetVsHardwareType();
            ImGui::Text( "PPU: %d, Hardware: %d", vsPpu, vsHardware );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Vs. system type info (Byte 13)." );
            ImGui::EndGroup();
        }

        // Miscellaneous ROMs and Default Expansion Device
        {
            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Misc ROMs:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int miscRoms = iNes.GetMiscRoms();
            ImGui::Text( "%d", miscRoms );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Miscellaneous ROM count (Byte 14)." );
            ImGui::EndGroup();

            ImGui::BeginGroup();
            ImGui::PushFont( renderer->fontMonoBold );
            ImGui::Text( "Default Expansion:" );
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Indent( innerSpacing );
            int defExp = iNes.GetDefaultExpansionDevice();
            ImGui::Text( "%d", defExp );
            ImGui::SameLine();
            ImGui::Indent( outerSpacing );
            HelpMarker( "Default expansion device (Byte 15)." );
            ImGui::EndGroup();
        }

        ImGui::EndChild();
        ImGui::EndChild();
    }

    void RenderMenuBar()
    {
        if ( ImGui::BeginMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Close" ) ) {
                    visible = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    static void BinaryButtons( int id, u8 *value )
    {
        /*
           @brief: 8 series of buttons representing a byte. Clicking each toggles the given bit and adjusts
           the value accordingly
        */

        // Button, no padding, no bg color, subtle hover color
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.1, 0.1, 0.1, 0.1 ) );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.2, 0.2, 0.2, 0.2 ) );

        ImVec2 buttonSize = ImVec2( 10, 16 );
        for ( int i = 7; i >= 0; i-- ) {
            bool bit = ( *value & ( 1 << i ) ) != 0;
            if ( i != 7 ) {
                ImGui::SameLine( 0, 0 );
            }
            int offset = id * 1000;
            ImGui::PushID( i + offset );
            if ( ImGui::Button( std::to_string( bit ).c_str(), buttonSize ) ) {
                *value ^= 1 << i;
            }
            ImGui::PopID();
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor( 3 );
    }
};
