#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class OverlayWindow : public UIComponent
{
public:
  OverlayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

  void OnVisible() override {}
  void OnHidden() override {}

  void RenderSelf() override
  {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const float          pad = 10.0f;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 const         workPos = viewport->WorkPos;
    ImVec2 const         workSize = viewport->WorkSize;
    ImVec2 const         windowPos = ImVec2( workPos.x + workSize.x - pad, workPos.y + pad );
    ImVec2 const         windowPosPivot = ImVec2( 1.0f, 0.0f );
    ImGui::SetNextWindowPos( windowPos, ImGuiCond_Always, windowPosPivot );
    ImGui::SetNextWindowViewport( viewport->ID );
    windowFlags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha( 0.75f );

    if ( ImGui::Begin( "Emulator Overlay", &visible, windowFlags ) ) {
      ImGui::PushFont( renderer->fontMono );
      ImGui::Text( "Cycle: " U64_FORMAT_SPECIFIER, renderer->bus.cpu.GetCycles() );
      ImGui::Text( "CyclePS: %.1f", renderer->GetCyclesPerSecond() );
      ImGui::Text( "FPS: %.1f", renderer->GetAvgFps() );
      ImGui::Text( "Frame Count: " U64_FORMAT_SPECIFIER, renderer->bus.ppu.frame );
      ImGui::PopFont();
    }
    ImGui::End();
  }
};
