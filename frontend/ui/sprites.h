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
  SpritesWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

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
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::SetNextWindowSizeConstraints( ImVec2( 490, 850 ), ImVec2( 490, 850 ) );

    if ( ImGui::Begin( "Sprite Viewer", &visible, windowFlags ) ) {
      RenderMenuBar();
      DebugControls( "Sprite Debugger" );

      ImGui::BeginChild( "sprite window top", ImVec2( 410, 220 ) );
      ImGui::PushFont( renderer->fontMono );
      LeftPanel();
      ImGui::SameLine();
      RightPanel();
      ImGui::PopFont();
      ImGui::EndChild();

      float  scale = 1.8f;
      ImVec2 spriteWindowSize = ImVec2( 256 * scale, 240 * scale );
      ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::Spectrum::GRAY200 );
      ImGui::BeginChild( "sprite window bottom", ImVec2( spriteWindowSize ), ImGuiChildFlags_Borders,
                         ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar );
      ImGui::PopStyleColor();
      RenderScreenSprites( scale );

      ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }

  void LeftPanel()
  {
    ImVec2 const           panelSize = ImVec2( 212, 212 );
    ImVec2 const           tilemapSize = ImVec2( 192, 192 );
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::Spectrum::GRAY100 );
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
    ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
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

        CustomComponents::selectable( nullptr, spriteIdx, cellSize, cellSelected, cellHovered, ImVec4( 0, 0, 0, 0 ),
                                      ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ), ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ), onHover );

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

  void RenderScreenSprites( float scale = 1 )
  {
    ImVec2       childPos = ImGui::GetCursorScreenPos();
    ImDrawList  *drawList = ImGui::GetWindowDrawList();
    GLuint const textureHandle = renderer->GrabOamTextureHandle();
    ImTextureID  textureID = (ImTextureID) (intptr_t) textureHandle;

    const float atlasSize = 64.0f;
    const float tileSize = 8.0f;

    for ( int cellIdx = 0; cellIdx < 64; cellIdx++ ) {
      SpriteEntry sprite = ppu.GetOamEntry( cellIdx );
      if ( sprite.y >= 240 )
        continue; // Skip off-screen sprites

      // UV coordinates based on cell index.
      int    col = cellIdx % 8;
      int    row = cellIdx / 8;
      float  u0 = ( static_cast<float>( col * static_cast<int>( tileSize ) ) ) / atlasSize;
      float  v0 = ( static_cast<float>( row * static_cast<int>( tileSize ) ) ) / atlasSize;
      float  u1 = ( static_cast<float>( ( col + 1 ) * static_cast<int>( tileSize ) ) ) / atlasSize;
      float  v1 = ( static_cast<float>( ( row + 1 ) * static_cast<int>( tileSize ) ) ) / atlasSize;
      ImVec2 uv0( u0, v0 );
      ImVec2 uv1( u1, v1 );

      // Adjust for mirroring.
      if ( sprite.attribute.bit.flipH ) {
        std::swap( uv0.x, uv1.x );
      }
      if ( sprite.attribute.bit.flipV ) {
        std::swap( uv0.y, uv1.y );
      }

      // Scale the destination rectangle.
      ImVec2 posMin( childPos.x + ( static_cast<float>( sprite.x ) * scale ),
                     childPos.y + ( static_cast<float>( sprite.y ) * scale ) );
      ImVec2 posMax( posMin.x + ( tileSize * scale ), posMin.y + ( tileSize * scale ) );

      drawList->AddImage( textureID, posMin, posMax, uv0, uv1 );

      // Draw a border if this cell is selected.
      if ( cellSelected == cellIdx ) {
        float  t = static_cast<float>( ImGui::GetTime() );
        float  delta = 0.5f + ( 0.5f * sinf( t * 5.0f ) );
        ImVec2 pMin = posMin;
        ImVec2 pMax = posMax;
        // For a border, no need to adjust inner rect if you only want one outline.
        ImVec4 colorA( 1.0f, 1.0f, 1.0f, 1.0f );
        ImVec4 colorB( 0.5f, 0.5f, 0.5f, 1.0f );
        ImVec4 tweenedColor;
        tweenedColor.x = colorA.x * ( 1.0f - delta ) + colorB.x * delta;
        tweenedColor.y = colorA.y * ( 1.0f - delta ) + colorB.y * delta;
        tweenedColor.z = colorA.z * ( 1.0f - delta ) + colorB.z * delta;
        tweenedColor.w = colorA.w * ( 1.0f - delta ) + colorB.w * delta;
        ImU32 tweenedColor32 =
            IM_COL32( static_cast<int>( tweenedColor.x * 255 ), static_cast<int>( tweenedColor.y * 255 ),
                      static_cast<int>( tweenedColor.z * 255 ), static_cast<int>( tweenedColor.w * 255 ) );

        drawList->AddRect( pMin, pMax, tweenedColor32, 0.0f, 0, 1.0f );
      }
    }
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
