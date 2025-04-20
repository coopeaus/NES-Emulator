#pragma once
#include <cmath>
#include <functional>
#include <imgui.h>

/*
################################
#                              #
#            Helpers           #
#                              #
################################
*/

inline ImU32 contrastColor( ImU32 color )
{
  /*
    @brief: Returns a contrasting color based on the input color.
  */
  ImVec4 const bgColorVec = ImGui::ColorConvertU32ToFloat4( color );
  float const  luminance = ( 0.299f * bgColorVec.x ) + ( 0.587f * bgColorVec.y ) + ( 0.114f * bgColorVec.z );
  return luminance > 0.5f ? IM_COL32_BLACK : IM_COL32_WHITE;
}

inline ImU32 contrastColor( ImVec4 color )
{
  /*
    @brief: Returns a contrasting color based on the input color.
  */
  float const luminance = ( 0.299f * color.x ) + ( 0.587f * color.y ) + ( 0.114f * color.z );
  return luminance > 0.5f ? IM_COL32_BLACK : IM_COL32_WHITE;
}

/*
################################
#                              #
#       Custom Components      #
#                              #
################################
*/

namespace CustomComponents
{
// clang-format off
inline bool selectable(
    const char *label, 
    int cellIdx, 
    const ImVec2 cellSize, 
    int &cellSelected, 
    int &cellHovered,
    ImVec4 bgColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.3f ), 
    ImVec4 hoverColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.7f ),
    ImVec4 mouseDownColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.5f ), 
    const std::function<void()> &onHover = []() {},
    const std::function<void()> &onClick = []() {} )
{
  // clang-format on

  /*
    @brief: Custom selectable component that allows for custom styling and callbacks.

    @details: This component wraps ImGui::Button. There is a native ImGui::Selectable, but its customization
    is limited. This component is intended to be used in grid-like layouts, where a give cell can be
    selected and hovered.

    @param label: Label to display. Pass nullptr to not display a label.
    @param cellIdx: Unique value for a given cell.
    @param cellSize: Size of the cell.
    @param cellSelected: Reference to the currently selected cell.
    @param cellHovered: Reference to the currently hovered cell.
    @param bgColor: Background color. Set to transparent to not display a background.
    @param hoverColor: Hover color
    @param mouseDownColor: Mouse down color
    @param onHover: Default behavior sets the cellHovered value. Additional callback can be passed, which
    does not override the hover behavior
    @param onClick: Default behavior sets the cellSelected value. Additional callback can be passed, which
    does not override the click behavior

    @return: Returns true if the cell was clicked.

   */

  bool const isSelected = ( cellSelected == cellIdx );
  ImGui::PushID( cellIdx );

  // Override styles
  ImGui::PushStyleColor( ImGuiCol_Button, bgColor );
  ImGui::PushStyleColor( ImGuiCol_ButtonHovered, hoverColor );
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, mouseDownColor );
  ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

  bool const clicked = ImGui::Button( "##cell", cellSize );

  ImGui::PopStyleColor( 3 );
  ImGui::PopStyleVar();

  ImDrawList *drawList = ImGui::GetWindowDrawList();

  if ( clicked ) {
    cellSelected = cellIdx;
    onClick();
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

    drawList->AddRect( pMin, pMax, IM_COL32( 0, 0, 0, 255 ), 0.0f, 0, 2.0f );
    drawList->AddRect( pMin, pMax, tweenedColor32, 0.0f, 0, 2.0f );
    drawList->AddRect( pInnerMin, pInnerMax, IM_COL32( 0, 0, 0, 255 ), 0.0f, 0, 2.0f );
  }

  if ( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay ) ) {
    cellHovered = cellIdx;
    onHover();
  }

  if ( label != nullptr ) {

    ImVec2 const pMin = ImGui::GetItemRectMin();
    ImVec2 const pMax = ImGui::GetItemRectMax();
    ImU32 const  textColor = contrastColor( bgColor );
    ImVec2 const textPos = ImVec2( pMin.x + 4, pMin.y + 2 );
    drawList->AddText( textPos, textColor, label );
  }

  ImGui::PopID();

  return clicked;
}

inline float getLuminance( ImVec4 color )
{
  return ( 0.299f * color.x ) + ( 0.587f * color.y ) + ( 0.114f * color.z );
}

} // namespace CustomComponents
