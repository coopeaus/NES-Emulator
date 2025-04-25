#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class Notification : public UIComponent
{
public:
  Notification( Renderer *renderer ) : UIComponent( renderer ) { visible = renderer->messageShow; }

  void OnVisible() override { visible = renderer->messageShow; }
  void OnHidden() override { visible = renderer->messageShow; }

  void RenderSelf() override
  {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

    const float          pad = 10.0f;
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImVec2 const         workPos = viewport->WorkPos;
    ImVec2 const         workSize = viewport->WorkSize;
    // bottom-left corner: x = left + pad, y = bottom - pad
    ImVec2 const windowPos = ImVec2( workPos.x + pad, workPos.y + workSize.y - pad );
    ImVec2 const windowPosPivot = ImVec2( 0.0f, 1.0f );

    ImGui::SetNextWindowPos( windowPos, ImGuiCond_Always, windowPosPivot );
    ImGui::SetNextWindowViewport( viewport->ID );
    windowFlags |= ImGuiWindowFlags_NoMove;
    ImGui::SetNextWindowBgAlpha( 0.75f );

    if ( ImGui::Begin( "Notification Overlay", &visible, windowFlags ) ) {
      ImGui::TextUnformatted( renderer->message.c_str() );
    }
    ImGui::End();
  }
};
