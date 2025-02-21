
#include "renderer.h"
#include <imgui.h>
inline void overlay( Renderer *renderer, UI *ui ) // NOLINT
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    // Position window in the top-right corner
    const float          pad = 10.0F;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImVec2 workPos = viewport->WorkPos;
    ImVec2 workSize = viewport->WorkSize;
    ImVec2 windowPos = ImVec2( workPos.x + workSize.x - pad, workPos.y + pad );
    ImVec2 windowPosPivot = ImVec2( 1.0F, 0.0F );
    ImGui::SetNextWindowPos( windowPos, ImGuiCond_Always, windowPosPivot );
    ImGui::SetNextWindowViewport( viewport->ID );
    windowFlags |= ImGuiWindowFlags_NoMove;

    // Transparent background
    ImGui::SetNextWindowBgAlpha( 0.35F );
    if ( ImGui::Begin( "Overlay", &ui->showOverlay, windowFlags ) ) {
        ImGui::PushFont( ui->renderer->fontMono );
        ImGui::Text( "CPU Cycle: %lld", renderer->bus.cpu.GetCycles() );
        ImGui::Text( "FPS(%.1f FPS)", renderer->io->Framerate );
        ImGui::PopFont();
    }
    ImGui::End();
}
