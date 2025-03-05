#pragma once
#include "bus.h"
#include "ui-component.h"
#include "renderer.h"
#include <cstdio>
#include <imgui.h>
#include "custom-components.h"
#include <imgui-spectrum.h>
#include <string>

class PaletteWindow : public UIComponent
{
  public:
    PaletteWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    /*
    ################################
    #           Variables          #
    ################################
    */
    int systemColorSelected = 0;
    int systemColorHovered = 0;
    int ppuColorSelected = 0;
    int ppuColorHovered = 0;

    enum TabType : int { SYSTEM, PPU };
    int tabSelected = PPU;

    /*
    ################################
    #            Methods           #
    ################################
    */

    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 620, 400 ), ImVec2( 1000, 600 ) );

        if ( ImGui::Begin( "Palettes", &visible, windowFlags ) ) {
            RenderMenuBar();

            ImGui::PushFont( renderer->fontMono );

            // Left Panel
            LeftPanel();

            ImGui::SameLine();

            // Right Panel
            RightPanel();
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    void LeftPanel()
    {
        ImGui::PushStyleColor( ImGuiCol_ChildBg, Spectrum::GRAY100 );
        ImGui::BeginChild( "left panel", ImVec2( 330, 0 ), ImGuiChildFlags_Borders );
        RenderTabs();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void RightPanel()
    {
        ImGui::BeginChild( "right panel" );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Properties" );
        ImGui::PopFont();
        ImGui::Separator();

        if ( tabSelected == SYSTEM ) {
            SystemProps( systemColorSelected );
            PaletteSwap();
        } else {
            PpuProps( ppuColorSelected );
            PaletteSwap();
        }

        ImGui::EndChild();
    }

    void PaletteSwap()
    {

        ImGui::AlignTextToFramePadding();
        ImGui::Text( "System Palette:" );
        ImGui::SameLine();

        ImGui::BeginDisabled( renderer->bus.ppu.failedPaletteRead );
        ImGui::Text( "%d", renderer->bus.ppu.systemPaletteIdx );
        if ( ImGui::Button( "<" ) ) {
            renderer->bus.ppu.DecrementSystemPalette();
        }
        ImGui::SameLine();
        if ( ImGui::Button( ">" ) ) {
            renderer->bus.ppu.IncrementSystemPalette();
        }
        ImGui::EndDisabled();
    }

    void SystemProps( int targetId, float indentSpacing = 140 )
    {
        ImGui::BeginGroup();
        ImGui::Text( "Index" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "$%02X (%d)", targetId, targetId );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Color (Hex)" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%s", Rgba32ToHexString( renderer->bus.ppu.GetMasterPaletteColor( targetId ) ) );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Color (RGB)" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        u32 const colorInt = renderer->bus.ppu.GetMasterPaletteColor( targetId );
        u8 const  r = static_cast<u8>( colorInt & 0xFF );
        u8 const  g = static_cast<u8>( colorInt >> 8 ) & 0xFF;
        u8 const  b = static_cast<u8>( colorInt >> 16 ) & 0xFF;
        ImGui::Text( "(%d, %d, %d)", r, g, b );

        ImGui::EndGroup();
    }

    void PpuProps( int targetId, float indentSpacing = 140 )
    {
        {
            ImGui::BeginGroup();
            u16 const paletteAddress = 0x3F00 + targetId;
            u8 const  colorIndex = renderer->bus.ppu.Read( paletteAddress );

            ImGui::Text( "Index" );
            ImGui::SameLine();
            ImGui::Indent( indentSpacing );
            ImGui::Text( "%s", std::to_string( targetId ).c_str() );

            ImGui::Unindent( indentSpacing );
            ImGui::Text( "PPU Address" );
            ImGui::SameLine();
            ImGui::Indent( indentSpacing );
            ImGui::Text( "$%04X", paletteAddress );

            ImGui::Unindent( indentSpacing );
            ImGui::Text( "Value" );
            ImGui::SameLine();
            ImGui::Indent( indentSpacing );
            ImGui::Text( "$%02X (%d)", colorIndex, colorIndex );

            ImGui::Unindent( indentSpacing );
            ImGui::Text( "Color (Hex)" );
            ImGui::SameLine();
            ImGui::Indent( indentSpacing );
            ImGui::Text( "%s", Rgba32ToHexString( renderer->bus.ppu.GetMasterPaletteColor( colorIndex ) ) );

            ImGui::Unindent( indentSpacing );
            ImGui::Text( "Color (RGB)" );
            ImGui::SameLine();
            ImGui::Indent( indentSpacing );
            u32 const colorInt = renderer->bus.ppu.GetMasterPaletteColor( colorIndex );
            u8 const  r = static_cast<u8>( colorInt & 0xFF );
            u8 const  g = static_cast<u8>( colorInt >> 8 ) & 0xFF;
            u8 const  b = static_cast<u8>( colorInt >> 16 ) & 0xFF;
            ImGui::Text( "(%d, %d, %d)", r, g, b );

            ImGui::EndGroup();
        }
    }
    void RenderTabs()
    {
        ImGuiTabBarFlags const tabBarFlags = ImGuiTabBarFlags_None;
        if ( ImGui::BeginTabBar( "PaletteTabs", tabBarFlags ) ) {

            if ( ImGui::BeginTabItem( "PPU" ) ) {
                tabSelected = PPU;
                RenderPpuPaletteGrid( ImVec2( 30, 30 ) );
                ImGui::EndTabItem();
            }

            if ( ImGui::BeginTabItem( "System" ) ) {
                tabSelected = SYSTEM;
                RenderSystemPaletteGrid( ImVec2( 30, 30 ) );
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void RenderPpuPaletteGrid( ImVec2 cellSize )
    {
        ImGui::Dummy( ImVec2( 5, 5 ) );
        for ( int rowStart = 0; rowStart < 32; rowStart += 4 ) {
            ImGui::Dummy( ImVec2( 0, 0 ) );
            for ( int cell = 0; cell < 4; cell++ ) {
                ImGui::SameLine( 0.0f, 0.0f );
                int    cellIdx = rowStart + cell;
                u16    paletteAddress = 0x3F00 + cellIdx;
                u8     colorIndex = renderer->bus.ppu.Read( paletteAddress );
                ImVec4 paletteColor = Rgba32ToImVec4( renderer->bus.ppu.GetMasterPaletteColor( colorIndex ) );
                char   label[3];
                snprintf( label, sizeof( label ), "%02X", colorIndex );

                auto onHover = [&]() {
                    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
                    if ( ImGui::BeginItemTooltip() ) {
                        PpuProps( cellIdx, 120 );
                        ImGui::EndTooltip();
                    }
                    ImGui::PopStyleVar();
                };
                // clang-format off
                CustomComponents::selectable( label, cellIdx, cellSize, ppuColorSelected, ppuColorHovered,
                                              paletteColor, 
                                              HighlightColor( paletteColor, 0.2 ),
                                              HighlightColor( paletteColor, 0.4 ), 
                                              onHover );
                // clang-format on
            }
        }
    }

    static ImVec4 HighlightColor( ImVec4 color, float factor )
    {
        float luminance = static_cast<float>( 0.299 * color.x + 0.587 * color.y + 0.114 * color.z );

        // darken or lighten based on luminance
        if ( luminance > 0.5f ) {
            color.x -= factor;
            color.y -= factor;
            color.z -= factor;
        } else {
            color.x += factor;
            color.y += factor;
            color.z += factor;
        }

        return color;
    }

    void RenderSystemPaletteGrid( ImVec2 cellSize )
    {
        ImGui::Dummy( ImVec2( 5, 5 ) );
        for ( int rowStart = 0; rowStart < 64; rowStart += 8 ) {
            ImGui::Dummy( ImVec2( 0, 0 ) );
            for ( int cell = 0; cell < 8; cell++ ) {
                ImGui::SameLine( 0.0f, 0.0f );
                int cellIdx = rowStart + cell;

                auto onHover = [&]() {
                    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
                    if ( ImGui::BeginItemTooltip() ) {
                        SystemProps( cellIdx, 120 );
                        ImGui::EndTooltip();
                    }
                    ImGui::PopStyleVar();
                };

                char label[3];
                snprintf( label, sizeof( label ), "%02X", rowStart + cell );
                ImVec4 paletteColor =
                    Rgba32ToImVec4( renderer->bus.ppu.GetMasterPaletteColor( rowStart + cell ) );

                // clang-format off
                CustomComponents::selectable( label, cellIdx, cellSize, systemColorSelected, systemColorHovered, 
                                              paletteColor, 
                                              HighlightColor( paletteColor, 0.2 ),
                                              HighlightColor( paletteColor, 0.4 ), 
                                              onHover );
            }
        }
    }

    static ImColor Rgba32ToImColor( u32 color )
    {
        int const r = static_cast<int>( color & 0xFF );
        int const g = static_cast<int>( color >> 8 ) & 0xFF;
        int const b = static_cast<int>( color >> 16 ) & 0xFF;
        return { r, g, b };
    }

    static ImVec4 Rgba32ToImVec4( u32 color )
    {
        float r = static_cast<float>( color & 0xFF ) / 255.0f;
        float g = static_cast<float>( ( color >> 8 ) & 0xFF ) / 255.0f;
        float b = static_cast<float>( ( color >> 16 ) & 0xFF ) / 255.0f;

        return { r, g, b, 255 };
    }

    static const char *Rgba32ToHexString( u32 color )
    {
        u8 const r = static_cast<u8>( color & 0xFF );
        u8 const g = static_cast<u8>( color >> 8 ) & 0xFF;
        u8 const b = static_cast<u8>( color >> 16 ) & 0xFF;

        static char hexString[8]; // 1 hash, 6 hex digits, 1 null terminator
        snprintf( hexString, sizeof( hexString ), "$%02X%02X%02X", r, g, b );
        return hexString;
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
