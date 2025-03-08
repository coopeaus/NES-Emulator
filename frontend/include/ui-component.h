#pragma once

// Define the right format specifier for u64 based on the platform
#include <imgui.h>
#ifdef __APPLE__
#define U64_FORMAT_SPECIFIER "%llu"
#else
#define U64_FORMAT_SPECIFIER "%lu"
#endif

class Renderer;

class UIComponent
{
  public:
    UIComponent( const UIComponent & ) = default;
    UIComponent( UIComponent && ) = delete;
    UIComponent &operator=( const UIComponent & ) = default;
    UIComponent &operator=( UIComponent && ) = delete;
    virtual ~UIComponent() = default;

    UIComponent( Renderer *renderer ) : renderer( renderer ) {}

    virtual void OnVisible() = 0;
    virtual void OnHidden() = 0;
    virtual void RenderSelf() = 0;

    void Show() { visible = true; }
    void Hide() { visible = false; }

    bool visible{};

    enum DebuggerStatus : int {
        NORMAL,
        PAUSED,
        STEPPING,
        TIMEOUT,
        RESET,
    };
    DebuggerStatus debuggerStatus = NORMAL;

    void DebugControls();

    static void HelpMarker( const char *desc )
    {
        ImGui::TextDisabled( "(?)" );
        if ( ImGui::BeginItemTooltip() ) {
            ImGui::PushTextWrapPos( ImGui::GetFontSize() * 35.0f );
            ImGui::TextUnformatted( desc );
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

  protected:
    Renderer *renderer;
};
