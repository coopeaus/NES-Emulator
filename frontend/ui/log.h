#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

struct ExampleAppLog;

class LogWindow : public UIComponent // NOLINT
{
  public:
    uint64_t lastCpuCycleLogged = 0;
    LogWindow( Renderer *renderer ) : UIComponent( renderer )
    {
        visible = true;
        renderer->bus.cpu.EnableTracelog();
    }

    ~LogWindow() override { renderer->bus.cpu.DisableTracelog(); }

    // if ( ImGui::SmallButton( "[Debug] Add 5 entries" ) ) {
    //     static int  counter = 0;
    //     const char *categories[3] = { "info", "warn", "error" };
    //     const char *words[] = { "Bumfuzzled",   "Cattywampus", "Snickersnee", "Abibliophobia",
    //                             "Absquatulate", "Nincompoop",  "Pauciloquent" };
    //     for ( int n = 0; n < 5; n++ ) {
    //         const char *category = categories[counter % IM_ARRAYSIZE( categories )];
    //         const char *word = words[counter % IM_ARRAYSIZE( words )];
    //         AddLog( "[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
    //                 ImGui::GetFrameCount(), category, ImGui::GetTime(), word );
    //         counter++;
    //     }
    // }

    void RenderSelf() override
    {
        ImGui::SetNextWindowSize( ImVec2( 500, 400 ), ImGuiCond_FirstUseEver );
        ImGui::Begin( "Trace Log", &visible );
        uint64_t currentCycle = renderer->bus.cpu.GetCycles();
        if ( currentCycle != lastCpuCycleLogged ) {
            lastCpuCycleLogged = currentCycle;
            for ( const auto &line : renderer->bus.cpu.GetTracelog() ) {
                std::cout << "Logging CPU trace log\n";
                AddLog( "%s\n", line.c_str() );
            }
        }
        ImGui::End();

        Draw( "Trace Log", &visible );
    }

  private:
    ImGuiTextBuffer _buf;
    ImGuiTextFilter _filter;
    ImVector<int>   _lineOffsets;        // Index to lines offset. We maintain this with AddLog() calls.
    bool            _autoScroll{ true }; // Keep scrolling if already at the bottom.

    void Clear()
    {
        _buf.clear();
        _lineOffsets.clear();
        _lineOffsets.push_back( 0 );
        renderer->bus.cpu.ClearTracelog();
    }

    void AddLog( const char *fmt, ... ) IM_FMTARGS( 2 )
    {
        int     oldSize = _buf.size();
        va_list args = nullptr;
        va_start( args, fmt );
        _buf.appendfv( fmt, args );
        va_end( args );
        for ( int newSize = _buf.size(); oldSize < newSize; oldSize++ ) {
            if ( _buf[oldSize] == '\n' ) {
                _lineOffsets.push_back( oldSize + 1 );
            }
        }
    }

    void Draw( const char *title, bool *pOpen = nullptr ) // NOLINT
    {
        if ( !ImGui::Begin( title, pOpen ) ) {
            ImGui::End();
            return;
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
        bool clear = ImGui::Button( "Clear" );
        ImGui::SameLine();
        bool copy = ImGui::Button( "Copy" );
        ImGui::SameLine();
        _filter.Draw( "Filter", -100.0f );

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
            const char *buf = _buf.begin();
            const char *bufEnd = _buf.end();
            if ( _filter.IsActive() ) {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store
                // the result of search/filter.. especially if the filtering function is not trivial (e.g.
                // reg-exp).
                for ( int lineNo = 0; lineNo < _lineOffsets.Size; lineNo++ ) {
                    const char *lineStart = buf + _lineOffsets[lineNo];
                    const char *lineEnd =
                        ( lineNo + 1 < _lineOffsets.Size ) ? ( buf + _lineOffsets[lineNo + 1] - 1 ) : bufEnd;
                    if ( _filter.PassFilter( lineStart, lineEnd ) ) {
                        ImGui::TextUnformatted( lineStart, lineEnd );
                    }
                }
            } else {
                // The simplest and easy way to display the entire buffer:
                //   ImGui::TextUnformatted(buf_begin, buf_end);
                // And it'll just work. TextUnformatted() has specialization for large blob of text and
                // will fast-forward to skip non-visible lines. Here we instead demonstrate using the
                // clipper to only process lines that are within the visible area. If you have tens of
                // thousands of items and their processing cost is non-negligible, coarse clipping them on
                // your side is recommended. Using ImGuiListClipper requires
                // - A) random access into your data
                // - B) items all being the  same height,
                // both of which we can handle since we have an array pointing to the beginning of each
                // line of text. When using the filter (in the block of code above) we don't have random
                // access into the data to display anymore, which is why we don't use the clipper. Storing
                // or skimming through the search result would make it possible (and would be recommended
                // if you want to search through tens of thousands of entries).
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

            // Keep up at the bottom of the scroll region if we were already at the bottom at the
            // beginning of the frame. Using a scrollbar or mouse-wheel will take away from the bottom
            // edge.
            if ( _autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() ) {
                ImGui::SetScrollHereY( 1.0f );
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }
};
