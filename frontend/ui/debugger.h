#pragma once
#include "ui-component.h"
#include "renderer.h"
#include "log.h"
#include <imgui.h>
#include <chrono>

class DebuggerWindow : public UIComponent
{
  public:
    DebuggerWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}

    // globals
    enum DebuggerStatus : u8 {
        NORMAL,
        PAUSED,
        STEPPING,
        TIMEOUT,
    };
    DebuggerStatus debuggerStatus = NORMAL;

    void RenderSelf() override // NOLINT
    {
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 430, -1 ), ImVec2( 600, -1 ) );

        if ( ImGui::Begin( "Debugger", &visible, windowFlags ) ) {
            RenderMenuBar();
            DebugControls(); // Defined in ui-component.cpp
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    void RenderMenuBar()
    {
        if ( ImGui::BeginMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Close" ) ) {
                    visible = false;
                }
                ImGui::EndMenu();
            }
            if ( ImGui::BeginMenu( "Debug" ) ) {
                if ( ImGui::MenuItem( "Reset" ) ) {
                    renderer->bus.DebugReset();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }
};
