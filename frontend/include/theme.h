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
} // namespace CustomTheme

// clang-format on
// NOLINTEND
