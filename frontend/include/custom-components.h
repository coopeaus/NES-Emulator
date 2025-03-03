#pragma once
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
    ImVec4 const bgColorVec = ImGui::ColorConvertU32ToFloat4( color );
    float const luminance = ( 0.299f * bgColorVec.x ) + ( 0.587f * bgColorVec.y ) + ( 0.114f * bgColorVec.z );
    return luminance > 0.5f ? IM_COL32_BLACK : IM_COL32_WHITE;
}

inline void selectableColor( ImU32 color )
{
    ImVec2 const pMin = ImGui::GetItemRectMin();
    ImVec2 const pMax = ImGui::GetItemRectMax();
    ImGui::GetWindowDrawList()->AddRectFilled( pMin, pMax, color );
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
inline bool selectable( const char *label, bool *pSelected, ImU32 bgColor, ImGuiSelectableFlags flags,
                        const ImVec2 &sizeArg )
{
    /* @brief ImGui::Selectable wrapper that allows bgColor and label color control
     */

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

inline bool selectableNoBg( bool *pSelected, ImGuiSelectableFlags flags, const ImVec2 &sizeArg,
                            const char *label = nullptr )
{
    /* @brief ImGui::Selectable wrapper that doesn't draw the background.
       Can be used as overlay over images or other UI.
     */

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

    // drawList->ChannelsSetCurrent( 0 );
    // selectableColor( IM_COL32( 0, 0, 0, 0 ) );

    drawList->ChannelsSetCurrent( 1 );
    if ( *pSelected ) {
        ImVec2 const pMin = ImGui::GetItemRectMin();
        ImVec2 const pMax = ImGui::GetItemRectMax();
        ImU32 const  borderColor = IM_COL32( 255, 255, 255, 255 );
        // Draw a faint white fill and a solid black border when selected.
        drawList->AddRectFilled( pMin, pMax, IM_COL32( 255, 0, 255, 100 ) );
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

} // namespace CustomComponents
