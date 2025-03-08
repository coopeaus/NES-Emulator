#pragma once
#include "ui-component.h"
#include "renderer.h"
#include "log.h"
#include <imgui.h>
#include <chrono>

class DebuggerWindow : public UIComponent
{
  public:
    DebuggerWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

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
        ImGui::SetNextWindowSizeConstraints( ImVec2( 300, -1 ), ImVec2( 400, -1 ) );

        if ( ImGui::Begin( "Debugger", &visible, windowFlags ) ) {
            RenderMenuBar();
            bool const isPaused = renderer->paused;

            ImGui::BeginDisabled( !isPaused );
            ImGui::PushItemWidth( 140 );
            if ( ImGui::Button( "Continue" ) ) {
                renderer->paused = false;
                debuggerStatus = NORMAL;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled( isPaused );
            if ( ImGui::Button( "Pause" ) ) {
                renderer->paused = true;
                debuggerStatus = PAUSED;
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

            const char *items[] = { "Cycles", "Instructions", "VBlank", "Scanlines", "Frame", "NMI", "IRQ" };
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
                debuggerStatus = STEPPING;

                using Clock = std::chrono::steady_clock;
                auto didTimeout = [&]() -> bool {
                    auto const elapsed =
                        std::chrono::duration<float>( Clock::now() - renderer->frameStart ).count();

                    if ( elapsed > 2.0 ) {
                        debuggerStatus = TIMEOUT;
                    }

                    return debuggerStatus == TIMEOUT;
                };

                switch ( item ) {
                    case 0: { // Cycles
                        auto const target = renderer->bus.cpu.GetCycles() + i0;
                        while ( renderer->bus.cpu.GetCycles() < target && !didTimeout() ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    }
                    case 1: // Instructions
                        for ( int i = 0; i < i0; i++ ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    case 2: // VBlank
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
                    case 3: { // Scanlines
                        auto const target = renderer->bus.ppu.GetScanline() + i0;
                        while ( renderer->bus.ppu.GetScanline() < target ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    }
                    case 4: { // Frame
                        auto const target = renderer->bus.ppu.GetFrame() + i0;
                        while ( renderer->bus.ppu.GetFrame() < target ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        break;
                    }
                    case 5: { // NMI
                        if ( !renderer->bus.ppu.GetCtrlNmiEnable() ) {
                            while ( !renderer->bus.ppu.GetCtrlNmiEnable() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        } else {
                            while ( renderer->bus.ppu.GetCtrlNmiEnable() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                            while ( !renderer->bus.ppu.GetCtrlNmiEnable() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        }
                        break;
                    } break;
                    case 6: // IRQ
                        if ( !renderer->bus.cpu.GetInterruptDisableFlag() ) {
                            while ( !renderer->bus.cpu.GetInterruptDisableFlag() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        } else {
                            while ( renderer->bus.cpu.GetInterruptDisableFlag() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                            while ( !renderer->bus.cpu.GetInterruptDisableFlag() && !didTimeout() ) {
                                renderer->bus.cpu.DecodeExecute();
                            }
                        }
                        break;
                    default:
                        break;
                }
            }

            ImGui::Dummy( ImVec2( 0, 10 ) );

            switch ( debuggerStatus ) {
                case TIMEOUT:
                    ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
                    ImGui::Text( "Timed out! Step condition could not be matched." );
                    ImGui::PopStyleColor();
                    break;

                default:
                    ImGui::Text( "" );
                    break;
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
