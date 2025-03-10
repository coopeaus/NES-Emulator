#pragma once
#include "bus.h"
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>
#include <cinttypes>

class CpuViewerWindow : public UIComponent
{
  public:
    CPU &cpu; // NOLINT
    CpuViewerWindow( Renderer *renderer ) : UIComponent( renderer ), cpu( renderer->bus.cpu )
    {
        visible = false;
    }

    /*
    ################################
    #           Variables          #
    ################################
    */

    /*
    ################################
    #            Methods           #
    ################################
    */
    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override
    {
        constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 420, 500 ), ImVec2( 420, 500 ) );

        if ( ImGui::Begin( "CPU Viewer", &visible, windowFlags ) ) {
            RenderMenuBar();
            DebugControls();
            ImGui::Spacing();
            ImGui::PushFont( renderer->fontMono );

            CpuRegisters();
            CpuStatus();

            ImGui::Spacing();

            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    void CpuRegisters()
    {
        ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ImGui::BeginChild( "registers", ImVec2( 0, 160 ), ImGuiChildFlags_Borders );

        float const innerSpacing = 25.0f;
        float const outerSpacing = 45.0f;

        // display the registers types accordingly
        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "A:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetAccumulator() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "X:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetXRegister() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Y:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetYRegister() );

        ImGui::EndGroup();

        // New row
        ImGui::BeginGroup();

        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "PC:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetProgramCounter() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "SP:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetStackPointer() );

        ImGui::SameLine();
        ImGui::Indent( outerSpacing );
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "P:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( innerSpacing );
        ImGui::Text( "%02X", cpu.GetStatusRegister() );
        ImGui::EndGroup();

        ImGui::Dummy( ImVec2( 10, 10 ) );
        ImGui::Separator();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "CPU Cycle: " );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( 100 );
        ImGui::Text( U64_FORMAT_SPECIFIER, cpu.GetCycles() );
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "PPU Cycle: " );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( 100 );
        ImGui::Text( "%d", renderer->bus.ppu.GetCycles() );
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Scanline:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( 100 );
        ImGui::Text( "%d", renderer->bus.ppu.GetScanline() );
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Frame:" );
        ImGui::PopFont();
        ImGui::SameLine();
        ImGui::Indent( 100 );
        ImGui::Text( U64_FORMAT_SPECIFIER, renderer->bus.ppu.GetFrame() );
        ImGui::EndGroup();

        ImGui::PopStyleColor();
        ImGui::EndChild();
    }

    void CpuStatus() const
    {

        ImGui::SeparatorText( "Status" );

        // get status value
        u8 const status = cpu.GetStatusRegister();

        // check each flag
        bool carryBool = ( status & CPU::Status::Carry ) != 0;
        bool zeroBool = ( status & CPU::Status::Zero ) != 0;
        bool interruptBool = ( status & CPU::Status::InterruptDisable ) != 0;
        bool decimalBool = ( status & CPU::Status::Decimal ) != 0;
        bool brkBool = ( status & CPU::Status::Break ) != 0;
        bool unusedBool = ( status & CPU::Status::Unused ) != 0;
        bool overflowBool = ( status & CPU::Status::Overflow ) != 0;
        bool negativeBool = ( status & CPU::Status::Negative ) != 0;

        float const statusSpacing = 140.0f;

        // status check boxes
        ImGui::BeginGroup();
        ImGui::BeginDisabled( !carryBool );
        ImGui::Checkbox( "Carry", &carryBool );
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::Indent( statusSpacing );
        ImGui::BeginDisabled( !zeroBool );
        ImGui::Checkbox( "Zero", &zeroBool );
        ImGui::EndDisabled();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::BeginDisabled( !interruptBool );
        ImGui::Checkbox( "Interrupt", &interruptBool );
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::Indent( statusSpacing );
        ImGui::BeginDisabled( !decimalBool );
        ImGui::Checkbox( "Decimal", &decimalBool );
        ImGui::EndDisabled();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::BeginDisabled( !brkBool );
        ImGui::Checkbox( "Break", &brkBool );
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::Indent( statusSpacing );
        ImGui::BeginDisabled( !unusedBool );
        ImGui::Checkbox( "Unused", &unusedBool );
        ImGui::EndDisabled();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::BeginDisabled( !overflowBool );
        ImGui::Checkbox( "Overflow", &overflowBool );
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::Indent( statusSpacing );
        ImGui::BeginDisabled( !negativeBool );
        ImGui::Checkbox( "Negative", &negativeBool );
        ImGui::EndDisabled();
        ImGui::EndGroup();
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
            ImGui::EndMenuBar();
        }
    }
};
