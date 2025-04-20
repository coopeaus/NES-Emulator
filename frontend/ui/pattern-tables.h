#pragma once
#include "bus.h"
#include "imgui-spectrum.h"
#include "ui-component.h"
#include "custom-components.h"
#include "renderer.h"
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <imgui.h>
#include <string>

class PatternTablesWindow : public UIComponent
{
public:
  PatternTablesWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

  /*
  ################################
  #           Variables          #
  ################################
  */
  enum TabType : int { Chr };
  int tabSelected = Chr;
  int cellSelected = 0;
  int cellHovered = 0;
  int paletteCellSelected = 0;
  int paletteCellHovered = 0;

  /*
  ################################
  #            Methods           #
  ################################
  */

  void OnVisible() override { renderer->updatePatternTables = true; }
  void OnHidden() override { renderer->updatePatternTables = false; }
  void RenderSelf() override
  {
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::SetNextWindowSizeConstraints( ImVec2( 400, 800 ), ImVec2( 600, 800 ) );

    if ( ImGui::Begin( "Pattern Table Viewer", &visible, windowFlags ) ) {
      RenderMenuBar();
      DebugControls( "Pattern Table Debugger" );

      ImGui::PushFont( renderer->fontMono );
      LeftPanel();
      ImGui::SameLine();
      RightPanel();
      ImGui::PopFont();
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }
  void RenderTabs()
  {
    ImGuiTabBarFlags const tabBarFlags = ImGuiTabBarFlags_None;

    if ( ImGui::BeginTabBar( "PaletteTabs", tabBarFlags ) ) {

      if ( ImGui::BeginTabItem( "Chr ROM" ) ) {
        tabSelected = Chr;
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }

  void LeftPanel()
  {
    ImVec2 const           panelSize = ImVec2( 276, 600 );
    ImVec2 const           tilemapSize = ImVec2( 256, 256 );
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::Spectrum::GRAY100 );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::BeginChild( "pattern table panel", panelSize, ImGuiChildFlags_Borders, windowFlags );

    ImGui::Text( "Pattern Table 0" );
    RenderPatternTable( 0, tilemapSize );
    ImGui::Separator();

    ImGui::Spacing();

    ImGui::Text( "Pattern Table 1" );
    RenderPatternTable( 1, tilemapSize, 256 );

    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
  }

  void RightPanel()
  {
    ImGui::BeginChild( "patter table properties", ImVec2( 0, 0 ) );

    ImGui::PushFont( renderer->fontMonoBold );
    ImGui::Text( "Properties" );
    ImGui::PopFont();
    ImGui::Separator();
    PatternTableProps( cellSelected );
    ImGui::EndChild();
  }

  void RenderPatternTable( int tableIdx, ImVec2 parentSize, int idOffset = 0 )
  {
    ImVec2 const           windowSize = ImVec2( parentSize.x, parentSize.y );
    ImVec2 const           tilesetSize = windowSize;
    ImVec2 const           cellSize = ImVec2( tilesetSize.x / 16, tilesetSize.y / 16 );
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
    char childName[32];
    snprintf( childName, sizeof( childName ), "PatternTable%d", tableIdx );
    ImGui::BeginChild( childName, windowSize, 0, windowFlags );

    // Grab texture
    GLuint const textureHandle = renderer->GrabPatternTableTextureHandle( tableIdx );
    ImGui::Image( (ImTextureID) (intptr_t) textureHandle, tilesetSize );

    // Save the current cursor position, for later restoring it.
    ImVec2 const currentCursorPos = ImGui::GetCursorScreenPos();
    ImVec2 const gridStartPos = ImGui::GetWindowPos();
    ImGui::SetCursorScreenPos( gridStartPos );

    ImDrawList *drawList = ImGui::GetWindowDrawList();

    for ( int rowStart = 0; rowStart < 256; rowStart += 16 ) {
      for ( int cellIdx = 0; cellIdx < 16; cellIdx++ ) {
        int const idx = ( rowStart + cellIdx + idOffset ) * 16;

        auto onHover = [&]() {
          ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
          if ( ImGui::BeginItemTooltip() ) {
            PatternTableProps( cellIdx, 120 );
            ImGui::EndTooltip();
          }
          ImGui::PopStyleVar();
        };

        CustomComponents::selectable( nullptr, idx, cellSize, cellSelected, cellHovered, ImVec4( 0, 0, 0, 0 ),
                                      ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ), ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ), onHover );
        if ( cellIdx < 15 ) {
          ImGui::SameLine( 0.0f, 0.0f );
        }
      }
    }

    // restore cursor
    ImGui::SetCursorScreenPos( currentCursorPos );
    ImGui::EndChild();
    ImGui::PopStyleVar();
  }

  bool GridCell( int cellIdx, const ImVec2 cellSize, int *currentSelection )
  {
    bool const isSelected = ( *currentSelection == cellIdx );
    ImGui::PushID( cellIdx );

    // Override styles
    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ) );
    ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

    bool const clicked = ImGui::Button( "##cell", cellSize );

    ImGui::PopStyleColor( 3 );
    ImGui::PopStyleVar();

    if ( clicked ) {
      *currentSelection = cellIdx;
    }

    if ( isSelected ) {
      float const t = static_cast<float>( ImGui::GetTime() );
      float const delta = 0.5f + ( 0.5f * sinf( t * 5.0f ) );

      // Get button rect
      ImVec2 const pMin = ImGui::GetItemRectMin();
      ImVec2 const pMax = ImGui::GetItemRectMax();
      // Get a slightly smaller rect
      ImVec2 const pInnerMin = ImVec2( pMin.x + 2, pMin.y + 2 );
      ImVec2 const pInnerMax = ImVec2( pMax.x - 2, pMax.y - 2 );

      // Interpolate between two colors, for easier visibility when selected
      ImVec4 const colorA = ImVec4( 1.0f, 1.0f, 1.0f, 1.0f );
      ImVec4 const colorB = ImVec4( 0.5f, 0.5f, 0.5f, 1.0f );
      ImVec4       tweenedColor;
      tweenedColor.x = colorA.x * ( 1.0f - delta ) + colorB.x * delta;
      tweenedColor.y = colorA.y * ( 1.0f - delta ) + colorB.y * delta;
      tweenedColor.z = colorA.z * ( 1.0f - delta ) + colorB.z * delta;
      tweenedColor.w = colorA.w * ( 1.0f - delta ) + colorB.w * delta;
      ImU32 const tweenedColor32 = IM_COL32( (int) ( tweenedColor.x * 255 ), (int) ( tweenedColor.y * 255 ),
                                             (int) ( tweenedColor.z * 255 ), (int) ( tweenedColor.w * 255 ) );

      ImDrawList *drawList = ImGui::GetWindowDrawList();
      drawList->AddRect( pMin, pMax, IM_COL32( 0, 0, 0, 255 ), 0.0f, 0, 4.0f );
      drawList->AddRect( pMin, pMax, tweenedColor32, 0.0f, 0, 2.0f );
      drawList->AddRect( pInnerMin, pInnerMax, IM_COL32( 0, 0, 0, 255 ), 0.0f, 0, 2.0f );
    }

    if ( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay ) ) {
      cellHovered = cellIdx;

      ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
      if ( ImGui::BeginItemTooltip() ) {
        PatternTableProps( cellIdx, 120 );
        ImGui::EndTooltip();
      }
      ImGui::PopStyleVar();
    }

    ImGui::PopID();

    return clicked;
  }

  static void PatternTableProps( int targetId, float indentSpacing = 140 )
  {
    u16 const tileAddress = 0x0000 + targetId;
    ImGui::BeginGroup();

    ImGui::Text( "Address (PPU)" );
    ImGui::SameLine();
    ImGui::Indent( indentSpacing );
    ImGui::Text( "$%04X", tileAddress );

    ImGui::Unindent( indentSpacing );
    ImGui::Text( "Tile Index" );
    ImGui::SameLine();
    ImGui::Indent( indentSpacing );
    int const tileIndex = targetId / 16;
    ImGui::Text( "$%02X (%d)", tileIndex, tileIndex );

    ImGui::EndGroup();
  }

  void Palettes()
  {
    for ( int rowStart = 0; rowStart < 32; rowStart += 4 ) {
      ImGui::Dummy( ImVec2( 0, 0 ) );
      for ( int cell = 0; cell < 4; cell++ ) {
        ImGui::SameLine();
        u32 const  colorIndex = rowStart + cell;
        u32 const  paletteColor = renderer->bus.ppu.GetPpuPaletteColor( colorIndex );
        bool const isSelected = paletteCellSelected == rowStart + cell;
        // TODO: Add palettes
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

  void PaletteProps( int targetId, float indentSpacing = 140 )
  {
    {
      ImGui::BeginGroup();
      u16 const paletteAddress = 0x3F00 + targetId;
      u8 const  colorIndex = renderer->bus.ppu.ReadVram( paletteAddress );

      ImGui::Text( "Index" );
      ImGui::SameLine();
      ImGui::Indent( indentSpacing );
      ImGui::Text( "%s", std::to_string( targetId ).c_str() );

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
