#pragma once
#include "ui-component.h"
#include "renderer.h"
#include "log.h"
#include <cstdint>
#include <imgui.h>

class DebuggerWindow : public UIComponent
{
  public:
    DebuggerWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 300, -1 ), ImVec2( 400, -1 ) );

        if ( ImGui::Begin( "Debugger", &visible, windowFlags ) ) {
            RenderMenuBar();
            bool const isPaused = renderer->paused;

            ImGui::BeginDisabled( !isPaused );
            ImGui::PushItemWidth( 140 );
            if ( ImGui::Button( "Continue" ) ) {
                renderer->paused = false;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled( isPaused );
            if ( ImGui::Button( "Pause" ) ) {
                renderer->paused = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            if ( ImGui::Button( "Reset" ) ) {
                if ( auto *logWindow = renderer->ui.GetComponent<LogWindow>() ) {
                    logWindow->Clear();
                }
                renderer->bus.DebugReset();
            }

            ImGui::SameLine();
            ImGui::Text( "CPU Cycle: " U64_FORMAT_SPECIFIER, renderer->bus.cpu.GetCycles() );
            ImGui::PopItemWidth();

            const char *items[] = { "Cycles", "Instructions", "VBlank", "Scanlines" };
            static int  i0 = 1;
            static int  item = 0;

            ImGui::Dummy( ImVec2( 0, 10 ) );
            ImGui::AlignTextToFramePadding();
            ImGui::Text( "Step" );
            ImGui::SameLine();
            ImGui::PushItemWidth( 120 );
            ImGui::InputInt( "", &i0 );
            ImGui::SameLine();
            ImGui::Combo( " ", &item, items, IM_ARRAYSIZE( items ) );
            ImGui::SameLine();
            ImGui::PopItemWidth();
            if ( ImGui::Button( "Go" ) ) {
                renderer->paused = true;
                switch ( item ) {
                    case 0: {
                        auto const target = renderer->bus.cpu.GetCycles() + i0;
                        while ( renderer->bus.cpu.GetCycles() < target ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    }
                    case 1:
                        for ( int i = 0; i < i0; i++ ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    case 2:
                        if ( !renderer->bus.ppu.GetStatusVblank() ) {
                            while ( !renderer->bus.ppu.GetStatusVblank() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        } else {
                            while ( renderer->bus.ppu.GetStatusVblank() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                            while ( !renderer->bus.ppu.GetStatusVblank() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        }
                        break;
                    case 3: {
                        auto const target = renderer->bus.ppu.GetScanline() + i0;
                        while ( renderer->bus.ppu.GetScanline() < target ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    bool _cycleBreakpoint = false;

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
