#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <cstdint>
#include <cstdarg>
#include <imgui.h>

#include <algorithm>

struct ExampleAppLog;

class LogWindow : public UIComponent // NOLINT
{
  public:
    uint64_t lastCpuCycleLogged = 0;
    LogWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override { renderer->bus.cpu.EnableTracelog(); }
    void OnHidden() override { renderer->bus.cpu.DisableTracelog(); }
    void RenderSelf() override // NOLINT
    {
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 1000, 600 ), ImVec2( 1000, 600 ) );
        if ( ImGui::Begin( "Trace Log", &visible, windowFlags ) ) {

            RenderMenuBar();
            DebugControls();

            // Grab the trace log as long as not on the same cycle.
            // We can debounce this more if necessary.
            uint64_t const currentCycle = renderer->bus.cpu.GetCycles();
            if ( currentCycle != lastCpuCycleLogged ) {
                _buf.clear();
                _lineOffsets.clear();
                _lineOffsets.push_back( 0 );
                lastCpuCycleLogged = currentCycle;
                for ( const auto &line : renderer->bus.cpu.GetTracelog() ) {
                    AddLog( line.c_str() );
                }
            }

            // Options menu
            if ( ImGui::BeginPopup( "Options" ) ) {
                ImGui::Checkbox( "Auto-scroll", &_autoScroll );
                ImGui::EndPopup();
            }

            // Main window
            if ( ImGui::Button( "Options" ) ) {
                ImGui::OpenPopup( "Options" );
            }
            ImGui::SameLine();
            bool const clear = ImGui::Button( "Clear" );
            ImGui::SameLine();
            bool const copy = ImGui::Button( "Copy" );
            ImGui::SameLine();
            ImGui::PushItemWidth( 120 );
            static int inputSize = renderer->bus.cpu.traceSize;
            if ( ImGui::InputInt( "Max Lines", &inputSize ) ) {
                inputSize = std::max( inputSize, 1 );
                inputSize = std::min( inputSize, 10000 );
                renderer->bus.cpu.traceSize = inputSize;
            }
            ImGui::PopItemWidth();

            ImGui::Dummy( ImVec2( 0, 10 ) );
            _filter.Draw( "Filter", -100.0f );
            ImGui::Dummy( ImVec2( 0, 10 ) );

            ImGui::Separator();

            if ( ImGui::BeginChild( "scrolling", ImVec2( 0, 0 ), ImGuiChildFlags_None,
                                    ImGuiWindowFlags_HorizontalScrollbar ) ) {
                if ( clear ) {
                    Clear();
                }
                if ( copy ) {
                    ImGui::LogToClipboard();
                }

                ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
                ImGui::PushFont( renderer->fontMono );
                const char *buf = _buf.begin();
                const char *bufEnd = _buf.end();

                // filter code pulled from imgui_demo, I have no idea how it works
                if ( _filter.IsActive() ) {
                    for ( int lineNo = 0; lineNo < _lineOffsets.Size; lineNo++ ) {
                        const char *lineStart = buf + _lineOffsets[lineNo];
                        const char *lineEnd = ( lineNo + 1 < _lineOffsets.Size )
                                                  ? ( buf + _lineOffsets[lineNo + 1] - 1 )
                                                  : bufEnd;
                        if ( _filter.PassFilter( lineStart, lineEnd ) ) {
                            ImGui::TextUnformatted( lineStart, lineEnd );
                        }
                    }
                } else {
                    ImGuiListClipper clipper;
                    clipper.Begin( _lineOffsets.Size );
                    while ( clipper.Step() ) {
                        for ( int lineNo = clipper.DisplayStart; lineNo < clipper.DisplayEnd; lineNo++ ) {
                            const char *lineStart = buf + _lineOffsets[lineNo];
                            const char *lineEnd = ( lineNo + 1 < _lineOffsets.Size )
                                                      ? ( buf + _lineOffsets[lineNo + 1] - 1 )
                                                      : bufEnd;
                            ImGui::TextUnformatted( lineStart, lineEnd );
                        }
                    }
                    clipper.End();
                }

                ImGui::PopStyleVar();
                ImGui::PopFont();

                // Keep up at the bottom of the scroll region if we were already at the bottom at the
                // beginning of the frame. Using a scrollbar or mouse-wheel will take away from the bottom
                // edge.
                if ( _autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() ) {
                    ImGui::SetScrollHereY( 1.0f );
                }
            }
            ImGui::EndChild();
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }

    void Clear()
    {
        _buf.clear();
        _lineOffsets.clear();
        _lineOffsets.push_back( 0 );
        renderer->bus.cpu.ClearTracelog();
    }

  private:
    ImGuiTextBuffer _buf;
    ImGuiTextFilter _filter;
    ImVector<int>   _lineOffsets;
    bool            _autoScroll{ true };

    void DebugControls()
    {
        // Debug transport controls
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
            renderer->bus.DebugReset();
            Clear();
        }

        ImGui::Separator();
        ImGui::Dummy( ImVec2( 0, 3 ) );
    }

    void RenderMenuBar()
    {
        if ( ImGui::BeginMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Close" ) ) {
                    visible = false;
                }
                ImGui::EndMenu();
            }
        }

        ImGui::EndMenuBar();
    }

    void AddLog( const char *str )
    {
        int oldSize = _buf.size();
        _buf.append( str );
        // Process new lines or any additional logic
        for ( int newSize = _buf.size(); oldSize < newSize; oldSize++ ) {
            if ( _buf[oldSize] == '\n' ) {
                _lineOffsets.push_back( oldSize + 1 );
            }
        }
    }
};
