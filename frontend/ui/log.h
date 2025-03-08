#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <cstdint>
#include <imgui.h>

#include <algorithm>

struct ExampleAppLog;

class LogWindow : public UIComponent // NOLINT
{
  public:
    uint64_t lastCpuCycleLogged = 0;
    LogWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

    void OnVisible() override
    {
        renderer->bus.cpu.EnableTracelog();
        renderer->bus.cpu.EnableMesenFormatTraceLog();
    }
    void OnHidden() override
    {
        renderer->bus.cpu.DisableTracelog();
        renderer->bus.cpu.DisableMesenFormatTraceLog();
    }

    // variables
    enum LogType : int {
        NORMAL, // Logs at the beginning of instruction
        MESEN,  // Matches mesen's output, logs each cpu cycle, between ppu cycle 2 and 3
    };
    int                       usingLogType = NORMAL;
    std::vector<const char *> logTypes = { "Normal", "Mesen" };

    void RenderSelf() override // NOLINT
    {
        if ( debuggerStatus == RESET ) {
            Clear();
        }

        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;

        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 920, 700 ), ImVec2( 920, 700 ) );
        if ( ImGui::Begin( "Trace Log", &visible, windowFlags ) ) {

            RenderMenuBar();
            DebugControls();
            ImGui::Dummy( ImVec2( 0, 10 ) );
            ImGui::PushItemWidth( 120 );
            ImGui::Combo( "Log Type", &usingLogType, logTypes.data(), (int) logTypes.size() );
            ImGui::PopItemWidth();
            ImGui::SameLine();
            HelpMarker( "Normal: Logs at the beginning of instruction. \nMesen: Logs each CPU cycle, between "
                        "PPU cycle 2 and 3." );
            ImGui::Dummy( ImVec2( 0, 10 ) );

            // Grab the trace log as long as not on the same cycle.
            // We can debounce this more if necessary.
            uint64_t const currentCycle = renderer->bus.cpu.GetCycles();
            if ( currentCycle != lastCpuCycleLogged ) {
                _buf.clear();
                _mesenBuf.clear();
                _lineOffsets.clear();
                _lineOffsets.push_back( 0 );
                lastCpuCycleLogged = currentCycle;
                for ( const auto &line : renderer->bus.cpu.GetTracelog() ) {
                    AddLog( line.c_str() );
                }
                for ( const auto &line : renderer->bus.cpu.GetMesenFormatTracelog() ) {
                    AddLogMesen( line.c_str() );
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

            ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
            if ( ImGui::BeginChild( "scrolling", ImVec2( 0, 0 ), ImGuiChildFlags_Border,
                                    ImGuiWindowFlags_HorizontalScrollbar ) ) {
                if ( clear ) {
                    Clear();
                }
                if ( copy ) {
                    ImGui::LogToClipboard();
                }

                ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
                ImGui::PushFont( renderer->fontMono );

                const char *buf = nullptr;
                const char *bufEnd = nullptr;
                if ( usingLogType == NORMAL ) {
                    buf = _buf.begin();
                    bufEnd = _buf.end();
                } else {
                    buf = _mesenBuf.begin();
                    bufEnd = _mesenBuf.end();
                }

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
            ImGui::PopStyleColor();
            ImGui::EndChild();
            ImGui::End();
            ImGui::PopStyleVar();
        }
    }

    void Clear()
    {
        _buf.clear();
        _mesenBuf.clear();
        _lineOffsets.clear();
        _lineOffsets.push_back( 0 );
        renderer->bus.cpu.ClearTraceLog();
        renderer->bus.cpu.ClearMesenTraceLog();
    }

  private:
    ImGuiTextBuffer _buf;
    ImGuiTextBuffer _mesenBuf;
    ImGuiTextFilter _filter;
    ImVector<int>   _lineOffsets;
    bool            _autoScroll{ true };

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
        for ( int const newSize = _buf.size(); oldSize < newSize; oldSize++ ) {
            if ( _buf[oldSize] == '\n' ) {
                _lineOffsets.push_back( oldSize + 1 );
            }
        }
    }

    void AddLogMesen( const char *str )
    {
        int oldSize = _mesenBuf.size();
        _mesenBuf.append( str );
        for ( int const newSize = _mesenBuf.size(); oldSize < newSize; oldSize++ ) {
            if ( _mesenBuf[oldSize] == '\n' ) {
                _lineOffsets.push_back( oldSize + 1 );
            }
        }
    }
};
