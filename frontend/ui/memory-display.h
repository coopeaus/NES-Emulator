#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>
#include <functional>

class MemoryDisplayWindow : public UIComponent
{
  public:
    MemoryDisplayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

    void RenderSelf() override
    {
        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 600, -1 ), ImVec2( 800, -1 ) );

        if ( ImGui::Begin( "Memory Viewer", &visible, windowFlags ) ) {

            // Used Bogdan's code for the continue/pause/reset logic as well as CPU cycle tracking
            RenderMenuBar();
            bool isPaused = renderer->paused;

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
            }

            ImGui::SameLine();
            ImGui::Text( "CPU Cycle: %lld", renderer->bus.cpu.GetCycles() );
            ImGui::PopItemWidth();

            // Add new items here
            const char *items[] = { "CPU Memory", "PPU Memory" };

            // Default to CPU Memory parameters
            static int         item_selected_idx = 0;
            static const char *header = "CPU Memory:";
            static const char *child_name = "CPU Memory";
            static int         lower_bound = 0;
            static int         upper_bound = 0xFFFF;
            static int         step = 16;
            // Use a lambda to capture the renderer instance
            static std::function<uint8_t( int )> read_func = [&]( int address ) -> uint8_t {
                return renderer->bus.cpu.Read( address );
            };

            // Build the ComboBox
            const char *combo_preview = items[item_selected_idx];
            if ( ImGui::BeginCombo( "Memory Type", combo_preview ) ) {
                for ( int n = 0; n < IM_ARRAYSIZE( items ); n++ ) {
                    const bool is_selected = ( item_selected_idx == n );
                    if ( ImGui::Selectable( items[n], is_selected ) ) {
                        item_selected_idx = n;
                    }
                    if ( is_selected ) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Assign parameters here for different memory types
            switch ( item_selected_idx ) {
                case 0:
                    header = "CPU Memory:";
                    child_name = "CPU Memory";
                    lower_bound = 0x0000;
                    upper_bound = 0xFFFF;
                    step = 16;
                    read_func = [&]( int address ) -> uint8_t { return renderer->bus.cpu.Read( address ); };
                    break;
                case 1:
                    header = "PPU Memory:";
                    child_name = "PPU Memory";
                    lower_bound = 0x0000;
                    upper_bound = 0x3FFF;
                    step = 16;
                    read_func = [&]( int address ) -> uint8_t { return renderer->bus.ppu.Read( address ); };
                    break;
                default:
                    break;
            }

            // Render the table using the above parameters
            RenderTable( header, child_name, lower_bound, upper_bound, step, read_func );
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    void RenderMenuBar()
    {
        if ( ImGui::BeginMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Exit" ) ) {
                    visible = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    /**
     * @brief Renders a memory table using ImGui.
     *
     * This function displays a table with memory addresses and their corresponding byte values.
     * The table includes a header, a child window, and columns for each byte.
     *
     * @param header The header text to display above the table.
     * @param child_name The name of the child window.
     * @param lower_bound The starting address of the memory range to display.
     * @param upper_bound The ending address of the memory range to display.
     * @param step The step size between memory addresses.
     * @param read_func A function that reads a byte from a given memory address.
     */
    void RenderTable( const char *header, const char *child_name, int lower_bound, int upper_bound, int step,
                      std::function<uint8_t( int )> read_func )
    {
        ImGui::Text( "%s", header );
        ImGui::BeginChild( child_name, ImVec2( 0, 300 ), true );

        // Use ImGui table to enforce column alignment
        if ( ImGui::BeginTable( "MemoryTable", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg ) ) {
            ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, 40 );

            // One column for each byte
            for ( int j = 0; j < 16; j++ ) {
                char colName[4];
                snprintf( colName, sizeof( colName ), "%02X", j );
                ImGui::TableSetupColumn( colName, ImGuiTableColumnFlags_WidthFixed, 20 );
            }
            ImGui::TableHeadersRow();

            // Memory display
            for ( int i = lower_bound; i <= upper_bound; i += step ) {
                ImGui::TableNextRow();

                // Address column
                ImGui::TableSetColumnIndex( 0 );
                ImGui::Text( "%04X", i );

                // Hex byte columns
                for ( int j = 0; j < 16; j++ ) {
                    uint8_t byte = read_func( i + j );

                    ImGui::TableSetColumnIndex( j + 1 );
                    ImGui::Text( "%02X", byte );
                }
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
};
