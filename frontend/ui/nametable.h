#pragma once
#include "bus.h"
#include "custom-components.h"
#include "imgui-spectrum.h"
#include "mappers/mapper-base.h"
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class NametableWindow : public UIComponent
{
public:
  NametableWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

  /*
  ################################
  #           Variables          #
  ################################
  */
  int cellSelected = 0;
  int cellHovered = 0;

  /*
  ################################
  #            Methods           #
  ################################
  */

  void OnVisible() override { renderer->updateNametables = true; }
  void OnHidden() override { renderer->updateNametables = false; }
  void RenderSelf() override
  {
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::SetNextWindowSizeConstraints( ImVec2( 1000, 820 ), ImVec2( 1000, 820 ) );

    if ( ImGui::Begin( "Nametable Viewer", &visible, windowFlags ) ) {
      RenderMenuBar();
      DebugControls( "Nametable Debugger" );

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
    ImVec2 const           panelSize = ImVec2( 660, 620 );
    ImVec2 const           tilemapSize = ImVec2( 320, 300 );
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::Spectrum::GRAY100 );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::BeginChild( "nametable panel", panelSize, ImGuiChildFlags_Borders, windowFlags );

    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 1 ) );
    RenderNametable( 0, tilemapSize );
    ImGui::SameLine( 0, 0 );
    ImGui::Dummy( ImVec2( 1, 0 ) );
    ImGui::SameLine( 0, 0 );
    RenderNametable( 1, tilemapSize, 0x0400 );
    RenderNametable( 2, tilemapSize, 0x0800 );
    ImGui::SameLine( 0, 0 );
    ImGui::Dummy( ImVec2( 1, 0 ) );
    ImGui::SameLine( 0, 0 );
    RenderNametable( 3, tilemapSize, 0x0C00 );
    ImGui::PopStyleVar();

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
  }

  void RightPanel()
  {
    ImGui::BeginChild( "nametable properties", ImVec2( 0, 0 ) );

    ImGui::PushFont( renderer->fontMonoBold );
    ImGui::Text( "Properties" );
    ImGui::PopFont();
    ImGui::Separator();
    NametableProps( cellSelected, 160 );
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

  void RenderNametable( int tableIdx, ImVec2 parentSize, int addrOffset = 0 )
  {
    ImVec2 const           windowSize = ImVec2( parentSize.x, parentSize.y );
    ImVec2 const           tilesetSize = windowSize;
    ImVec2 const           cellSize = ImVec2( tilesetSize.x / 32, tilesetSize.y / 30 );
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
    char childName[32];
    snprintf( childName, sizeof( childName ), "PatternTable%d", tableIdx );
    ImGui::BeginChild( childName, windowSize, 0, windowFlags );

    // Grab texture
    GLuint const textureHandle = renderer->GrabNametableTextureHandle( tableIdx );
    ImGui::Image( (ImTextureID) (intptr_t) textureHandle, tilesetSize );

    // Save the current cursor position, for later restoring it.
    ImVec2 const currentCursorPos = ImGui::GetCursorScreenPos();
    ImVec2 const gridStartPos = ImGui::GetWindowPos();
    ImGui::SetCursorScreenPos( gridStartPos );

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    u16 const baseTileAddr = 0x2000 + addrOffset;

    for ( int rowStart = 0; rowStart < 960; rowStart += 32 ) {
      for ( int cellIdx = 0; cellIdx < 32; cellIdx++ ) {
        int const tileAddr = baseTileAddr + rowStart + cellIdx;

        auto onHover = [&]() {
          ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
          if ( ImGui::BeginItemTooltip() ) {
            NametableProps( tileAddr, 160 );
            ImGui::EndTooltip();
          }
          ImGui::PopStyleVar();
        };

        CustomComponents::selectable( nullptr, tileAddr, cellSize, cellSelected, cellHovered, ImVec4( 0, 0, 0, 0 ),
                                      ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ), ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ), onHover );
        if ( cellIdx < 31 ) {
          ImGui::SameLine( 0.0f, 0.0f );
        }
      }
    }

    // restore cursor
    ImGui::SetCursorScreenPos( currentCursorPos );
    ImGui::EndChild();
    ImGui::PopStyleVar();
  }

  void NametableProps( int targetAddr, float indentSpacing = 140 )
  {
    ImGui::BeginGroup();

    ImGui::Text( "Tile Address (PPU)" );
    ImGui::SameLine();
    ImGui::Indent( indentSpacing );
    ImGui::Text( "$%04X", targetAddr );

    ImGui::Unindent( indentSpacing );
    ImGui::Text( "Tile Index" );
    ImGui::SameLine();
    ImGui::Indent( indentSpacing );
    int const tileValue = renderer->bus.ppu.ReadVram( targetAddr );
    ImGui::Text( "$%02X (%d)", tileValue, tileValue );

    ImGui::Unindent( indentSpacing );
    ImGui::Text( "Mirroring" );
    ImGui::SameLine();
    ImGui::Indent( indentSpacing );
    MirrorMode const mode = renderer->bus.ppu.GetMirrorMode();
    switch ( mode ) {
      case MirrorMode::Horizontal:
        ImGui::Text( "Horizontal" );
        break;
      case MirrorMode::Vertical:
        ImGui::Text( "Vertical" );
        break;
      case MirrorMode::SingleLower:
        ImGui::Text( "Single Lower" );
        break;
      case MirrorMode::SingleUpper:
        ImGui::Text( "Single Upper" );
        break;
      case MirrorMode::FourScreen:
        ImGui::Text( "Four Screen" );
        break;
    }

    ImGui::EndGroup();
  }
};
