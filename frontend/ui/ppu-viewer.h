#pragma once
#include "bus.h"
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>
#include <cinttypes>

class PpuViewerWindow : public UIComponent
{
  public:
    PpuViewerWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

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
        ImVec2 const               outerWindowSize = ImVec2( 650, 880 );
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( outerWindowSize,
                                             ImVec2( outerWindowSize.x + 100, outerWindowSize.y + 100 ) );

        if ( ImGui::Begin( "PPU Viewer", &visible, windowFlags ) ) {
            RenderMenuBar();
            DebugControls();
            ImGui::Spacing();
            ImGui::PushFont( renderer->fontMono );

            PpuCycles( ImVec2( 300, 140 ) );
            ImGui::SameLine();
            PpuStatus( ImVec2( 300, 140 ) );
            ImGui::Spacing();
            PpuCtrl( ImVec2( 300, 230 ) );
            ImGui::SameLine();
            PpuMask( ImVec2( 300, 230 ) );
            ImGui::Spacing();
            PpuOamAddr( ImVec2( 300, 140 ) );
            ImGui::SameLine();
            PpuVramAndScrolling( ImVec2( 300, 140 ) );
            ImGui::Spacing();
            PpuShifters( ImVec2( 300, 140 ) );

            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void PpuCycles( ImVec2 size )
    {
        SectionTableStart( "Cycles", size, 2 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "PPU Cycle" );
        ImGui::TableSetColumnIndex( 1 );
        int cycles = renderer->bus.ppu.GetCycles();
        ImGui::Text( "%d", cycles );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Scanline" );
        ImGui::TableSetColumnIndex( 1 );
        int scanline = renderer->bus.ppu.GetScanline();
        ImGui::Text( "%d", scanline );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Frame" );
        ImGui::TableSetColumnIndex( 1 );
        auto frame = renderer->bus.ppu.GetFrame();
        ImGui::Text( U64_FORMAT_SPECIFIER, frame );

        SectionTableEnd();
    }

    void PpuCtrl( ImVec2 size )
    {
        SectionTableStart( "PPUCTRL", size, 3 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "PPUCTRL" );
        ImGui::TableSetColumnIndex( 2 );
        auto ppuCtrl = renderer->bus.ppu.GetPpuCtrl();
        ImGui::Text( "$%02X", ppuCtrl );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.0" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Nametable X" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlNametableX() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.1" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Nametable Y" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlNametableY() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.2" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Inc Mode" );
        ImGui::TableSetColumnIndex( 2 );
        auto incMode = renderer->bus.ppu.GetCtrlIncrementMode();
        switch ( incMode ) {
            case 0:
                ImGui::Text( "$%02X (1)", 0 );
                break;
            case 1:
                ImGui::Text( "$%02X (32)", 1 );
                break;
            default:
                break;
        }

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.3" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Pattern Sprite" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlPatternSprite() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.4" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Pattern Bg" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlPatternBackground() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.5" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Sprite Size" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlSpriteSize() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2000.7" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "NMI Enable" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCtrlNmiEnable() );

        SectionTableEnd();
    }

    void PpuMask( ImVec2 size )
    {
        SectionTableStart( "PPUMASK", size, 3 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "PPUMASK" );
        ImGui::TableSetColumnIndex( 2 );
        auto ppuMask = renderer->bus.ppu.GetPpuMask();
        ImGui::Text( "$%02X", ppuMask );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.0" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Grayscale" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskGrayscale() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.1" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Show Bg Left" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskShowBgLeft() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.2" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Show Spr Left" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskShowSpritesLeft() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.3" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Show Bg" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskShowBg() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.4" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Show Spr" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskShowSprites() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.5" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Red Tint" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskEnhanceRed() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.6" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Green Tint" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskEnhanceGreen() );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2001.7" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Blue Tint" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetMaskEnhanceBlue() );

        SectionTableEnd();
    }

    void PpuOamAddr( ImVec2 size )
    {
        SectionTableStart( "OAMADDR", size, 3 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2003" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "OAMADDR" );
        ImGui::TableSetColumnIndex( 2 );
        auto oamAddr = renderer->bus.ppu.GetOamAddr();
        ImGui::Text( "$%02X", oamAddr );

        SectionTableEnd();
    }

    void PpuStatus( ImVec2 size )
    {
        SectionTableStart( "PPUSTATUS", size, 3 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2002" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "PPUSTATUS" );
        ImGui::TableSetColumnIndex( 2 );
        auto ppuStatus = renderer->bus.ppu.GetPpuStatus();
        ImGui::Text( "$%02X", ppuStatus );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2002.5" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Sprite Overflow" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetStatusSpriteOverflow() );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2002.6" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Sprite 0 Hit" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetStatusSpriteZeroHit() );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "$2002.7" );
        ImGui::TableSetColumnIndex( 1 );
        ImGui::Text( "Vblank" );
        ImGui::TableSetColumnIndex( 2 );
        ImGui::Text( "%d", renderer->bus.ppu.GetStatusVblank() );

        SectionTableEnd();
    }

    void PpuVramAndScrolling( ImVec2 size )
    {
        SectionTableStart( "VRAM and Scrolling", size, 2 );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "VRAM Addr" );
        ImGui::TableSetColumnIndex( 1 );
        auto vramAddr = renderer->bus.ppu.GetVramAddr();
        ImGui::Text( "$%04X", vramAddr );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Temp Addr" );
        ImGui::TableSetColumnIndex( 1 );
        auto tempAddr = renderer->bus.ppu.GetTempAddr();
        ImGui::Text( "$%04X", tempAddr );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Fine X" );
        ImGui::TableSetColumnIndex( 1 );
        auto fineX = renderer->bus.ppu.GetFineX();
        ImGui::Text( "$%02X", fineX );
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Addr Latch" );
        ImGui::TableSetColumnIndex( 1 );
        auto addrLatch = renderer->bus.ppu.GetAddrLatch();
        ImGui::Text( "$%02X", addrLatch );

        SectionTableEnd();
    }

    void PpuShifters( ImVec2 size )
    {

        SectionTableStart( "Shifters", size, 2 );

        ImGui::TableNextRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Bg Pattern Low" );
        ImGui::TableSetColumnIndex( 1 );
        auto bgPatternLow = renderer->bus.ppu.GetBgShiftPatternLow();
        ImGui::Text( "$%04X", bgPatternLow );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Bg Pattern High" );
        ImGui::TableSetColumnIndex( 1 );
        auto bgPatternHigh = renderer->bus.ppu.GetBgShiftPatternHigh();
        ImGui::Text( "$%04X", bgPatternHigh );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Bg Attr Low" );
        ImGui::TableSetColumnIndex( 1 );
        auto bgAttrLow = renderer->bus.ppu.GetBgShiftAttributeLow();
        ImGui::Text( "$%04X", bgAttrLow );

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex( 0 );
        ImGui::Text( "Bg Attr High" );
        ImGui::TableSetColumnIndex( 1 );
        auto bgAttrHigh = renderer->bus.ppu.GetBgShiftAttributeHigh();
        ImGui::Text( "$%04X", bgAttrHigh );

        SectionTableEnd();
    }

    static void SectionTableStart( const char *label, ImVec2 size, int columns )
    {
        static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingStretchSame |
                                       ImGuiTableFlags_Resizable | ImGuiTableFlags_Borders;
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ImGui::BeginChild( label, size, ImGuiChildFlags_Border );
        ImGui::PopStyleColor();
        ImGui::SeparatorText( label );
        ImGui::BeginTable( label, columns, flags );
    }

    static void SectionTableEnd()
    {
        ImGui::EndTable();
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
};
