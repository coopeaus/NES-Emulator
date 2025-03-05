#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>
#include "log.h"

class RegisterViewerWindow : public UIComponent
{
  public:
    RegisterViewerWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override
    {
        constexpr ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 300, -1 ), ImVec2( 400, -1 ) );

        if ( ImGui::Begin( "Register Viewer", &visible, windowFlags ) ) {
            auto &cpu = renderer->bus.cpu;

            bool const isPaused = renderer->paused;

            ImGui::BeginDisabled( !isPaused );
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
                if ( auto *logWindow = renderer->ui.GetComponent<LogWindow>() ) {
                    logWindow->Clear();
                }
            }

            // display the registers types accordingly
            ImGui::Text( "A:  %02X", cpu.GetAccumulator() );
            ImGui::Text( "X:  %02X", cpu.GetXRegister() );
            ImGui::Text( "Y:  %02X", cpu.GetYRegister() );
            ImGui::Text( "SP: %02X", cpu.GetStackPointer() );
            ImGui::Text( "P:  %02X", cpu.GetStatusRegister() );

            ImGui::Text( "PC: %04X", cpu.GetProgramCounter() );
            ImGui::Text( "Cycle: " U64_FORMAT_SPECIFIER, cpu.GetCycles() );

            // defined here due to being unable to access CPU declarations
            constexpr u8 carry = 1 << 0;
            constexpr u8 zero = 1 << 1;
            constexpr u8 decimal = 1 << 3;
            constexpr u8 brk = 1 << 4;
            constexpr u8 unused = 1 << 5;
            constexpr u8 overflow = 1 << 6;
            constexpr u8 negative = 1 << 7;

            // get status value
            u8 const status = cpu.GetStatusRegister();

            // check each flag
            bool carryBool = ( status & carry ) != 0;
            bool zeroBool = ( status & zero ) != 0;
            bool decimalBool = ( status & decimal ) != 0;
            bool brkBool = ( status & brk ) != 0;
            bool unusedBool = ( status & unused ) != 0;
            bool overflowBool = ( status & overflow ) != 0;
            bool negativeBool = ( status & negative ) != 0;

            // status check boxes
            ImGui::Checkbox( "Carry", &carryBool );
            ImGui::SameLine( 100 );

            ImGui::Checkbox( "Zero", &zeroBool );
            ImGui::SameLine( 200 );

            ImGui::Checkbox( "Decimal", &decimalBool );

            ImGui::Checkbox( "Break", &brkBool );
            ImGui::SameLine( 100 );

            ImGui::Checkbox( "Unused", &unusedBool );
            ImGui::SameLine( 200 );

            ImGui::Checkbox( "Overflow", &overflowBool );

            ImGui::Checkbox( "Negative", &negativeBool );
            ImGui::SameLine( 100 );
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
};
