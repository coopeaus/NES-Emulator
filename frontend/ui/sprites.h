#pragma once
#include "custom-components.h"
#include "imgui-spectrum.h"
#include "ui-component.h"
#include "renderer.h"
#include <cstdio>
#include <cstdint>
#include <imgui.h>
#include <string>

class SpritesWindow : public UIComponent
{
  public:
    SpritesWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

    /*
    ################################
    #           Variables          #
    ################################
    */
    int cellSelected = 0;
    int cellHovered = 0;
    int paletteCellSelected = 0;
    int paletteCellHovered = 0;

    PPU &ppu = renderer->bus.ppu;

    /*
    ################################
    #            Methods           #
    ################################
    */

    void OnVisible() override { renderer->updateOam = true; }
    void OnHidden() override { renderer->updateOam = false; }
    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 430, 400 ), ImVec2( 430, 400 ) );

        if ( ImGui::Begin( "Sprite Viewer", &visible, windowFlags ) ) {
            RenderMenuBar();
            DebugControls( "Sprite Debugger" );

            ImGui::PushFont( renderer->fontMono );
            LeftPanel();
            ImGui::SameLine();
            RightPanel();
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void LeftPanel()
    {
        ImVec2 const           panelSize = ImVec2( 212, 212 );
        ImVec2 const           tilemapSize = ImVec2( 192, 192 );
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

        ImGui::PushStyleColor( ImGuiCol_ChildBg, Spectrum::GRAY100 );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::BeginChild( "sprite panel", panelSize, ImGuiChildFlags_Borders, windowFlags );
        RenderOamSpriteData( 0, tilemapSize );

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void RightPanel()
    {
        ImGui::BeginChild( "sprite properties", ImVec2( 0, 0 ) );

        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Properties" );
        ImGui::PopFont();
        ImGui::Separator();
        PatternTableProps( cellSelected );
        ImGui::EndChild();
    }

    void RenderOamSpriteData( int tableIdx, ImVec2 parentSize )
    {
        ImVec2 const           windowSize = ImVec2( parentSize.x, parentSize.y );
        ImVec2 const           tilesetSize = windowSize;
        ImVec2 const           cellSize = ImVec2( tilesetSize.x / 8, tilesetSize.y / 8 );
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
        char childName[32];
        snprintf( childName, sizeof( childName ), "PatternTable%d", tableIdx );
        ImGui::BeginChild( childName, windowSize, 0, windowFlags );

        // Grab texture
        GLuint const textureHandle = renderer->GrabOamTextureHandle();
        ImGui::Image( (ImTextureID) (intptr_t) textureHandle, tilesetSize );

        // Save teh current cursor position, set new cursor pos on the top left
        ImVec2 const currentCursorPos = ImGui::GetCursorScreenPos();
        ImVec2 const gridStartPos = ImGui::GetWindowPos();
        ImGui::SetCursorScreenPos( gridStartPos );

        ImDrawList *drawList = ImGui::GetWindowDrawList();

        for ( int rowStart = 0; rowStart < 64; rowStart += 8 ) {
            for ( int col = 0; col < 8; col++ ) {
                int const spriteIdx = rowStart + col;

                auto onHover = [&]() {
                    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
                    if ( ImGui::BeginItemTooltip() ) {
                        PatternTableProps( spriteIdx, 110 );
                        ImGui::EndTooltip();
                    }
                    ImGui::PopStyleVar();
                };

                CustomComponents::selectable( nullptr, spriteIdx, cellSize, cellSelected, cellHovered,
                                              ImVec4( 0, 0, 0, 0 ), ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ),
                                              ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ), onHover );

                // Same line until the end of the row
                if ( col < 7 ) {
                    ImGui::SameLine( 0.0f, 0.0f );
                }
            }
        }

        ImGui::SetCursorScreenPos( currentCursorPos );
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    void PatternTableProps( int spriteIdx, float indentSpacing = 110 )
    {
        auto sprite = renderer->bus.ppu.GetOamEntry( spriteIdx );
        ImGui::BeginGroup();

        ImGui::Text( "Sprite Idx" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "$%02X", spriteIdx );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "X, Y" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%d, %d", sprite.x, sprite.y );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Size" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        std::string size = ppu.ppuCtrl.bit.spriteSize ? "8x16" : "8x8";
        ImGui::Text( "%s", size.c_str() );

        ImGui::Unindent( indentSpacing );
        ImGui::Separator();
        ImGui::Text( "Tile Idx" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%02X", sprite.tileIndex );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Palette Idx" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        auto paletteIdx = sprite.attribute.bit.palette;
        ImGui::Text( "$%02X", paletteIdx );

        ImGui::Unindent( indentSpacing );
        ImGui::Separator();
        ImGui::Text( "On Screen" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        bool isVisible = sprite.y < 240;
        ImGui::Text( "%s", isVisible ? "true" : "false" );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "H Mirror" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%d", sprite.attribute.bit.flipH );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "V Mirror" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%d", sprite.attribute.bit.flipV );

        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Priority" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "%s", sprite.attribute.bit.priority ? "Background" : "Foreground" );

        ImGui::EndGroup();
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
