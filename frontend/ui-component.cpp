#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

void UIComponent::DebugControls()
{
    ImVec2 size = ImVec2( 410, 120 );
    if ( ImGui::BeginChild( "debug-control", size, ImGuiChildFlags_Border ) ) {
        bool const isPaused = renderer->paused;

        ImGui::BeginDisabled( !isPaused );
        ImGui::PushItemWidth( 140 );
        if ( ImGui::Button( "Continue" ) ) {
            renderer->paused = false;
            debuggerStatus = NORMAL;
        }
        ImGui::PopItemWidth();
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
            debuggerStatus = RESET;
            renderer->bus.DebugReset();
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        HelpMarker( "H: PPU cycles, V: PPU scanline, C: CPU cycles" );
        ImGui::SameLine();
        ImGui::BeginGroup();
        float const innerSpacing = 20.0f;
        float const outerSpacing = 30.0f;

        ImGui::PushFont( renderer->fontMono );

        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "H:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%hu", renderer->bus.ppu.GetCycles() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "V:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%hd", renderer->bus.ppu.GetScanline() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "C:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( U64_FORMAT_SPECIFIER, renderer->bus.cpu.GetCycles() );

        ImGui::PopFont();
        ImGui::EndGroup();

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
                        while ( !renderer->bus.ppu.GetStatusVblank() && !didTimeout() ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                    } else {
                        while ( renderer->bus.ppu.GetStatusVblank() && !didTimeout() ) {
                            renderer->bus.cpu.DecodeExecute();
                        }
                        while ( !renderer->bus.ppu.GetStatusVblank() && !didTimeout() ) {

                            renderer->bus.cpu.DecodeExecute();
                        }
                    }
                    break;
                case 3: { // Scanlines
                    auto const target = renderer->bus.ppu.GetScanline() + i0;
                    while ( renderer->bus.ppu.GetScanline() < target && !didTimeout() ) {
                        renderer->bus.cpu.DecodeExecute();
                    }
                    break;
                }
                case 4: { // Frame
                    auto const target = renderer->bus.ppu.GetFrame() + i0;
                    while ( renderer->bus.ppu.GetFrame() < target && !didTimeout() ) {
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
                auto const line = renderer->bus.cpu.LogLineAtPC( false );
                ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
                ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 4.0f, 1.0f ) );
                if ( ImGui::BeginChild( "", ImVec2( 0, 0 ), ImGuiChildFlags_Border ) ) {
                    ;
                    ImGui::PushFont( renderer->fontMono );
                    ImGui::Text( "%s", line.c_str() );
                    ImGui::PopFont();
                    ImGui::EndChild();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                break;
        }

        ImGui::EndChild();
    }
}
