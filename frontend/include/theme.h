// Theme sourced from: https://github.com/GraphicsProgramming/dear-imgui-styles?tab=readme-ov-file
#pragma once
#include <imgui.h>
#include <imgui-spectrum.h>

// NOLINTBEGIN
// clang-format off
using namespace ImGui;

namespace CustomTheme {
inline void Style()
{
    ImGuiStyle *style = &ImGui::GetStyle();
    // style->FramePadding = ImVec2( 4, 6 );

    ImVec4 *colors = style->Colors;
    // colors[ImGuiCol_Text] = ColorConvertU32ToFloat4( Spectrum::GRAY800 );
    // colors[ImGuiCol_TextDisabled] = ColorConvertU32ToFloat4( Spectrum::GRAY500 );
    // colors[ImGuiCol_WindowBg] = ColorConvertU32ToFloat4( Spectrum::GRAY200 );
    // colors[ImGuiCol_ChildBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
    // colors[ImGuiCol_PopupBg] = ColorConvertU32ToFloat4( Spectrum::GRAY50 );
    // colors[ImGuiCol_Border] = ColorConvertU32ToFloat4( Spectrum::GRAY200 );
    // colors[ImGuiCol_BorderShadow] = ColorConvertU32ToFloat4( Spectrum::Static::NONE );
    // colors[ImGuiCol_FrameBg] = ColorConvertU32ToFloat4( Spectrum::GRAY75 );
    // colors[ImGuiCol_FrameBgHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY50 );
    // colors[ImGuiCol_FrameBgActive] = ColorConvertU32ToFloat4( Spectrum::GRAY200 );
    colors[ImGuiCol_TitleBg] = ColorConvertU32ToFloat4( Spectrum::Static::WHITE );
    colors[ImGuiCol_TitleBgActive] = ColorConvertU32ToFloat4( Spectrum::Static::WHITE );
    colors[ImGuiCol_TitleBgCollapsed] = ColorConvertU32ToFloat4( Spectrum::Static::WHITE );
    colors[ImGuiCol_Tab] = ColorConvertU32ToFloat4( Spectrum::GRAY300 );
    colors[ImGuiCol_TabHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    colors[ImGuiCol_TabSelected] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    // colors[ImGuiCol_MenuBarBg] = ColorConvertU32ToFloat4( Spectrum::GRAY200 );
    // colors[ImGuiCol_ScrollbarBg] = ColorConvertU32ToFloat4( Spectrum::GRAY100 );
    // colors[ImGuiCol_ScrollbarGrab] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    // colors[ImGuiCol_ScrollbarGrabHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY600 );
    // colors[ImGuiCol_ScrollbarGrabActive] = ColorConvertU32ToFloat4( Spectrum::GRAY700 );
    // colors[ImGuiCol_CheckMark] = ColorConvertU32ToFloat4( Spectrum::BLUE500 );
    // colors[ImGuiCol_SliderGrab] = ColorConvertU32ToFloat4( Spectrum::GRAY700 );
    // colors[ImGuiCol_SliderGrabActive] = ColorConvertU32ToFloat4( Spectrum::GRAY800 );
    // colors[ImGuiCol_Button] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    // colors[ImGuiCol_ButtonHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY50 );
    // colors[ImGuiCol_ButtonActive] = ColorConvertU32ToFloat4( Spectrum::GRAY200 );
    // colors[ImGuiCol_Header] = ColorConvertU32ToFloat4( Spectrum::Static::WHITE );
    // colors[ImGuiCol_HeaderHovered] = ColorConvertU32ToFloat4( Spectrum::BLUE500 );
    // colors[ImGuiCol_HeaderActive] = ColorConvertU32ToFloat4( Spectrum::BLUE600 );
    // colors[ImGuiCol_Separator] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    // colors[ImGuiCol_SeparatorHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY600 );
    // colors[ImGuiCol_SeparatorActive] = ColorConvertU32ToFloat4( Spectrum::GRAY700 );
    // colors[ImGuiCol_ResizeGrip] = ColorConvertU32ToFloat4( Spectrum::GRAY400 );
    // colors[ImGuiCol_ResizeGripHovered] = ColorConvertU32ToFloat4( Spectrum::GRAY600 );
    // colors[ImGuiCol_ResizeGripActive] = ColorConvertU32ToFloat4( Spectrum::GRAY700 );
    // colors[ImGuiCol_PlotLines] = ColorConvertU32ToFloat4( Spectrum::BLUE400 );
    // colors[ImGuiCol_PlotLinesHovered] = ColorConvertU32ToFloat4( Spectrum::BLUE600 );
    // colors[ImGuiCol_PlotHistogram] = ColorConvertU32ToFloat4( Spectrum::BLUE400 );
    // colors[ImGuiCol_PlotHistogramHovered] = ColorConvertU32ToFloat4( Spectrum::BLUE600 );
    // colors[ImGuiCol_TextSelectedBg] = ColorConvertU32ToFloat4( ( Spectrum::BLUE400 & 0x00FFFFFF ) | 0x33000000 );
    // colors[ImGuiCol_DragDropTarget] = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
    // colors[ImGuiCol_NavHighlight] = ColorConvertU32ToFloat4( ( Spectrum::GRAY900 & 0x00FFFFFF ) | 0x0A000000 );
    // colors[ImGuiCol_NavWindowingHighlight] = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
    // colors[ImGuiCol_NavWindowingDimBg] = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
    // colors[ImGuiCol_ModalWindowDimBg] = ImVec4( 0.20f, 0.20f, 0.20f, 0.35f );
}
// clang-format on

// NOLINTEND

// Forward declarations
ImU32 contrastColor( ImU32 color );

inline void selectableColor( ImU32 color )
{
    ImVec2 const pMin = ImGui::GetItemRectMin();
    ImVec2 const pMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled( pMin, pMax, color );
}

inline bool customSelectable( const char *label, bool *pSelected, ImU32 bgColor, ImGuiSelectableFlags flags,
                              const ImVec2 &sizeArg )
{
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit( 2 );

    // Channel number is like z-order. Widgets in higher channels are rendered above widgets in lower
    // channels.
    drawList->ChannelsSetCurrent( 1 );

    // Disable the default highlight, hover, active
    ImGui::PushStyleColor( ImGuiCol_Header, bgColor );
    ImGui::PushStyleColor( ImGuiCol_HeaderHovered, bgColor );
    ImGui::PushStyleColor( ImGuiCol_HeaderActive, bgColor );

    bool const result = ImGui::Selectable( "", pSelected, flags, sizeArg );

    ImGui::PopStyleColor( 3 );

    // Draw background
    drawList->ChannelsSetCurrent( 0 );
    selectableColor( bgColor );

    // Draw border for selected
    drawList->ChannelsSetCurrent( 1 );
    if ( *pSelected ) {
        ImVec2 const pMin = ImGui::GetItemRectMin();
        ImVec2 const pMax = ImGui::GetItemRectMax();
        ImU32 const  borderColor = contrastColor( bgColor );
        drawList->AddRect( pMin, pMax, borderColor, 0.0f, 0.0f, 1.0f );
    }

    // Draw label
    ImVec2 const pMin = ImGui::GetItemRectMin();
    ImVec2 const pMax = ImGui::GetItemRectMax();
    ImU32 const  textColor = contrastColor( bgColor );
    ImVec2 const textPos = ImVec2( pMin.x + 4, pMin.y + 2 );
    drawList->AddText( textPos, textColor, label );

    // Commit changes.
    drawList->ChannelsMerge();
    return result;
}

inline bool selectableTransparent( bool *pSelected, ImGuiSelectableFlags flags, const ImVec2 &sizeArg,
                                   const char *label = nullptr )
{
    // Remove extra padding so our cell matches exactly sizeArg.
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
    ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );

    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->ChannelsSplit( 2 );
    drawList->ChannelsSetCurrent( 1 );

    // Make the selectable background completely transparent.
    ImGui::PushStyleColor( ImGuiCol_Header, ImVec4( 0, 0, 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_HeaderHovered, ImVec4( 0, 0, 0, 0 ) );
    ImGui::PushStyleColor( ImGuiCol_HeaderActive, ImVec4( 0, 0, 0, 0 ) );

    bool const result = ImGui::Selectable( "", pSelected, flags, sizeArg );

    ImGui::PopStyleColor( 3 );

    drawList->ChannelsSetCurrent( 0 );
    // (Optional) If you have any background drawing, call it here.
    // selectableColor(IM_COL32(0, 0, 0, 0)); // Not needed if you want transparency.

    drawList->ChannelsSetCurrent( 1 );
    if ( *pSelected ) {
        ImVec2 const pMin = ImGui::GetItemRectMin();
        ImVec2 const pMax = ImGui::GetItemRectMax();
        ImU32 const  borderColor = IM_COL32( 255, 255, 255, 255 );
        // Draw a faint white fill and a solid black border when selected.
        drawList->AddRectFilled( pMin, pMax, IM_COL32( 255, 255, 255, 10 ) );
        drawList->AddRect( pMin, pMax, borderColor, 0.0f, 0, 1.0f );
    }

    if ( label ) {
        ImVec2 const pMin = ImGui::GetItemRectMin();
        ImU32 const  textColor = IM_COL32( 0, 0, 0, 255 );
        // Position the label with a slight offset.
        ImVec2 const textPos = ImVec2( pMin.x + 4, pMin.y + 2 );
        drawList->AddText( textPos, textColor, label );
    }

    // Pop both style variables.
    ImGui::PopStyleVar( 2 );
    drawList->ChannelsMerge();
    return result;
}

// Helpers
inline ImU32 contrastColor( ImU32 color )
{
    ImVec4 const bgColorVec = ColorConvertU32ToFloat4( color );
    float const luminance = ( 0.299f * bgColorVec.x ) + ( 0.587f * bgColorVec.y ) + ( 0.114f * bgColorVec.z );
    return luminance > 0.5f ? IM_COL32_BLACK : IM_COL32_WHITE;
}

} // namespace CustomTheme
