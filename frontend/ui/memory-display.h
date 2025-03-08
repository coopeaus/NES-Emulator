#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <cstdint>
#include <cstdio>
#include <imgui.h>
#include <functional>
#include "log.h"

class MemoryDisplayWindow : public UIComponent
{
  public:
    MemoryDisplayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}
    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 600, -1 ), ImVec2( 800, -1 ) );

        if ( ImGui::Begin( "Memory Viewer", &visible, windowFlags ) ) {

            RenderMenuBar();
            DebugControls();

            // Add new items here
            const char *items[] = { "CPU Memory", "PPU Memory" };

            // Default to CPU Memory parameters
            static int         itemSelectedIdx = 0;
            static const char *header = "CPU Memory:";
            static const char *childName = "CPU Memory";
            static int         lowerBound = 0;
            static int         upperBound = 0xFFFF;
            static int         step = 16;
            // Use a lambda to capture the renderer instance
            static std::function<uint8_t( int )> readFunc = [&]( int address ) -> uint8_t {
                return renderer->bus.cpu.Read( address );
            };

            // Build the ComboBox
            const char *comboPreview = items[itemSelectedIdx];
            if ( ImGui::BeginCombo( "Memory Type", comboPreview ) ) {
                for ( int n = 0; n < IM_ARRAYSIZE( items ); n++ ) {
                    const bool isSelected = ( itemSelectedIdx == n );
                    if ( ImGui::Selectable( items[n], isSelected ) ) {
                        itemSelectedIdx = n;
                    }
                    if ( isSelected ) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            // Assign parameters here for different memory types
            switch ( itemSelectedIdx ) {
                case 0:
                    header = "CPU Memory:";
                    childName = "CPU Memory";
                    lowerBound = 0x0000;
                    upperBound = 0xFFFF;
                    step = 16;
                    readFunc = [&]( int address ) -> uint8_t {
                        return renderer->bus.cpu.Read( address, true );
                    };
                    break;
                case 1:
                    header = "PPU Memory:";
                    childName = "PPU Memory";
                    lowerBound = 0x0000;
                    upperBound = 0x3FFF;
                    step = 16;
                    readFunc = [&]( int address ) -> uint8_t { return renderer->bus.ppu.Read( address ); };
                    break;
                default:
                    break;
            }

            // Render the table using the above parameters
            RenderTable( header, childName, lowerBound, upperBound, step, readFunc );
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
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

    /**
     * @brief Renders a memory table using ImGui.
     *
     * This function displays a table with memory addresses and their corresponding byte values.
     * The table includes a header, a child window, and columns for each byte.
     *
     * @param header The header text to display above the table.
     * @param childName The name of the child window.
     * @param lowerBound The starting address of the memory range to display.
     * @param upperBound The ending address of the memory range to display.
     * @param step The step size between memory addresses.
     * @param readFunc A function that reads a byte from a given memory address.
     */
    void RenderTable( const char *header, const char *childName, int lowerBound, int upperBound, int step,
                      const std::function<uint8_t( int )> &readFunc )
    {

        // TODO: Delete if not desired
        (void) header;
        // ImGui::Text( "%s", header );

        ImGui::BeginChild( childName, ImVec2( 0, 300 ), true );

        // Use ImGui table to enforce column alignment
        if ( ImGui::BeginTable( "MemoryTable", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg ) ) {
            ImGui::TableSetupColumn( "", ImGuiTableColumnFlags_WidthFixed, 40 );

            // One column for each byte
            ImGui::PushFont( renderer->fontMonoBold );

            // Set the background color for the header row
            for ( int j = 0; j < 16; j++ ) {
                char colName[4];
                snprintf( colName, sizeof( colName ), "%02X", j );
                ImGui::TableSetupColumn( colName, ImGuiTableColumnFlags_WidthFixed, 20 );
            }
            ImGui::TableHeadersRow();
            ImGui::PopFont();

            // Memory display
            for ( int i = lowerBound; i <= upperBound; i += step ) {
                ImGui::TableNextRow();

                // Address column
                ImGui::TableSetColumnIndex( 0 );
                ImGui::PushFont( renderer->fontMonoBold );
                ImGui::Text( "%04X", i );
                ImGui::PopFont();

                // Hex byte columns
                ImGui::PushFont( renderer->fontMono );
                ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 100, 100, 100, 255 ) );
                for ( int j = 0; j < 16; j++ ) {
                    uint8_t const byte = readFunc( i + j );

                    ImGui::TableSetColumnIndex( j + 1 );
                    ImGui::Text( "%02X", byte );
                }
                ImGui::PopStyleColor();
                ImGui::PopFont();
            }

            ImGui::EndTable();
        }

        ImGui::EndChild();
    }
};
