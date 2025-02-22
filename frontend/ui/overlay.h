#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class OverlayWindow : public UIComponent
{
  public:
    OverlayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

    void RenderSelf() override
    {
        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

        const float          pad = 10.0f;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImVec2               workPos = viewport->WorkPos;
        ImVec2               workSize = viewport->WorkSize;
        ImVec2               windowPos = ImVec2( workPos.x + workSize.x - pad, workPos.y + pad );
        ImVec2               windowPosPivot = ImVec2( 1.0f, 0.0f );
        ImGui::SetNextWindowPos( windowPos, ImGuiCond_Always, windowPosPivot );
        ImGui::SetNextWindowViewport( viewport->ID );
        windowFlags |= ImGuiWindowFlags_NoMove;
        ImGui::SetNextWindowBgAlpha( 0.35f );

        if ( ImGui::Begin( "Overlay", &visible, windowFlags ) ) {
            ImGui::PushFont( renderer->fontMono );
            ImGui::Text( "CPU Cycle: %lld", renderer->bus.cpu.GetCycles() );
            ImGui::Text( "FPS(%.1f FPS)", renderer->io->Framerate );
            ImGui::PopFont();
        }
        ImGui::End();
    }
};
