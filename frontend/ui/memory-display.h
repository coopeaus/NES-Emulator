#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <cstdint>
#include <cstdio>
#include <imgui.h>
#include <functional>

class MemoryDisplayWindow : public UIComponent
{
  public:
    MemoryDisplayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}

    // variables
    int  cellHovered = -1;
    int  cellSelected = -1;
    int  rowHovered = 0;
    int  columnHovered = 0;
    bool highlightRow = false;
    bool highlightCol = false;
    int  pcLocation = -1;

    enum MemorySpace : int {
        CPU, // Logs at the beginning of instruction
        PPU, // Matches mesen's output, logs each cpu cycle, between ppu cycle 2 and 3
    };
    int                       memorySpaceSelected = CPU;
    std::vector<const char *> memorySpaceLabels = { "CPU Memory", "PPU Memory" };

    enum CellType : int {
        ColumnHeader,
        RowHeader,
    };

    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 600, -1 ), ImVec2( 800, -1 ) );

        if ( ImGui::Begin( "Memory Viewer", &visible, windowFlags ) ) {

            RenderMenuBar();
            DebugControls( "Memory Viewer Debugger" );
            ImGui::Dummy( ImVec2( 0, 5 ) );

            // PC Location
            pcLocation = renderer->bus.cpu.GetProgramCounter();

            // Build the ComboBox
            if ( ImGui::Combo( "Memory Space", &memorySpaceSelected, memorySpaceLabels.data(),
                               (int) memorySpaceLabels.size() ) ) {
                cellSelected = -1;
                cellHovered = -1;
                highlightRow = false;
                highlightCol = false;
            }

            ImGui::Dummy( ImVec2( 0, 5 ) );

            // Assign parameters here for different memory types
            static int                           lowerBound = 0;
            static int                           upperBound = 0xFFFF;
            static int                           step = 16;
            static std::function<uint8_t( int )> readFunc = [&]( int address ) -> uint8_t {
                return renderer->bus.cpu.Read( address );
            };

            switch ( memorySpaceSelected ) {
                case CPU:
                    lowerBound = 0x0000;
                    upperBound = 0xFFFF;
                    step = 16;
                    readFunc = [&]( int address ) -> uint8_t {
                        return renderer->bus.cpu.Read( address, true );
                    };
                    break;
                case PPU:
                    lowerBound = 0x0000;
                    upperBound = 0x3FFF;
                    step = 16;
                    readFunc = [&]( int address ) -> uint8_t {
                        return renderer->bus.ppu.ReadVram( address );
                    };
                    break;
                default:
                    break;
            }

            // Render the table using the above parameters
            RenderTable( lowerBound, upperBound, step, readFunc );
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
     * @param lowerBound The starting address of the memory range to display.
     * @param upperBound The ending address of the memory range to display.
     * @param step The step size between memory addresses.
     * @param readFunc A function that reads a byte from a given memory address.
     */
    void RenderTable( int lowerBound, int upperBound, int step,
                      const std::function<uint8_t( int )> &readFunc )
    {

        const char *childName = memorySpaceLabels[memorySpaceSelected];
        ImGui::BeginChild( childName, ImVec2( 0, 400 ), true );

        // Use ImGui table to enforce column alignment
        static ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX |
                                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                            ImGuiTableFlags_ScrollY;
        if ( ImGui::BeginTable( "MemoryTable", 17, tableFlags ) ) {
            // Remove padding between cells
            ImGui::PushStyleVarY( ImGuiStyleVar_CellPadding, 0.0f );
            ImGui::TableSetupScrollFreeze( 0, 1 ); // Make top row always visible

            // Setup header columns, memory only, no labels
            for ( int j = 0; j < 17; j++ ) {
                ImGuiTableColumnFlags columnFlags =
                    ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHeaderLabel;
                if ( j == 0 ) {
                    ImGui::TableSetupColumn( "##h", columnFlags, 50 );
                    continue;
                }
                ImGui::TableSetupColumn( "##h", columnFlags, 30 );
            }
            ImGui::TableHeadersRow();

            // Push custom content to header columns
            ImGui::PushFont( renderer->fontMonoBold );
            for ( int j = 0; j < 17; j++ ) {
                ImGui::TableSetColumnIndex( j );
                if ( j == 0 ) {
                    BorderCell( j - 1, "##topleft", ColumnHeader );
                    continue;
                }
                char label[4];
                snprintf( label, sizeof( label ), "%02X", j - 1 );
                BorderCell( j - 1, label, ColumnHeader );
            }
            ImGui::PopFont();

            // Memory display
            for ( int i = lowerBound; i <= upperBound; i += step ) {
                ImGui::TableNextRow();

                // Address column
                ImGui::TableSetColumnIndex( 0 );
                ImGui::PushFont( renderer->fontMonoBold );
                char label[5];
                snprintf( label, sizeof( label ), "%04X", i );
                int borderCellIdx = i >> 4;
                BorderCell( borderCellIdx, label, RowHeader );
                ImGui::PopFont();

                // Hex byte columns
                ImGui::PushFont( renderer->fontMono );
                ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 100, 100, 100, 255 ) );
                for ( int j = 0; j < 16; j++ ) {
                    uint8_t const byte = readFunc( i + j );
                    ImGui::TableSetColumnIndex( j + 1 );
                    int cellIdx = i + j;
                    MemoryCell( cellIdx, byte );
                }
                ImGui::PopStyleColor();
                ImGui::PopFont();
            }

            ImGui::PopStyleVar();
            ImGui::EndTable();
        }

        ImGui::EndChild();
    }

    void MemoryCell( int cellIdx, int byte, ImVec4 bgColor = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ),
                     ImVec4 hoverColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.3f ),
                     ImVec4 mouseDownColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.6f ),
                     ImVec4 selectedColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.4f ),
                     ImVec4 pcLocationColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.4f ) )

    {

        ImGui::PushID( cellIdx + 0xFFFF );
        ImGui::PushStyleColor( ImGuiCol_Button, bgColor );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, hoverColor );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, mouseDownColor );
        int colorPushed = 3;
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

        // Highlight row
        if ( highlightRow && ( cellIdx >> 4 ) == rowHovered ) {
            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.4f, 0.6f, 0.9f, 0.1f ) );
            colorPushed++;
        }

        // Highlight column
        if ( highlightCol && ( cellIdx & 0xF ) == columnHovered ) {
            ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0.4f, 0.6f, 0.9f, 0.1f ) );
            colorPushed++;
        }

        // Selected
        if ( cellSelected == cellIdx ) {
            ImGui::PushStyleColor( ImGuiCol_Button, selectedColor );
            colorPushed++;
        }

        // PC location highlight
        if ( cellIdx == pcLocation ) {
            ImGui::PushStyleColor( ImGuiCol_Button, pcLocationColor );
            colorPushed++;
        }

        char label[4];
        snprintf( label, sizeof( label ), "%02X", byte );
        bool const clicked = ImGui::Button( label, ImVec2( -FLT_MIN, 0.0f ) );
        ImGui::PopID();
        ImGui::PopStyleColor( colorPushed );
        ImGui::PopStyleVar();

        if ( clicked ) {
            cellSelected = cellIdx;
        }

        // Hover
        if ( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_NoSharedDelay ) ) {
            cellHovered = cellIdx;
            rowHovered = cellIdx >> 4;
            columnHovered = cellIdx & 0xF;
            highlightRow = true;
            highlightCol = true;
            ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
            if ( ImGui::BeginItemTooltip() ) {
                CellProps( cellIdx, byte );
                ImGui::EndTooltip();
            }
            ImGui::PopStyleVar();
        }
    }

    void BorderCell( int cellIdx, const char *label, CellType type,
                     ImVec4 bgColor = ImVec4( 0.0f, 0.0f, 0.0f, 0.0f ),
                     ImVec4 hoverColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.3f ),
                     ImVec4 mouseDownColor = ImVec4( 0.4f, 0.6f, 0.9f, 0.6f ) )
    {

        ImGui::PushStyleColor( ImGuiCol_Button, bgColor );
        ImGui::PushStyleColor( ImGuiCol_ButtonHovered, hoverColor );
        ImGui::PushStyleColor( ImGuiCol_ButtonActive, mouseDownColor );
        ImGui::Button( label, ImVec2( -FLT_MIN, 0.0f ) );
        if ( ImGui::IsItemHovered( ImGuiHoveredFlags_DelayNone | ImGuiHoveredFlags_NoSharedDelay ) ) {
            if ( cellIdx >= 0 ) {
                if ( type == ColumnHeader ) {
                    columnHovered = cellIdx;
                    highlightCol = true;
                    highlightRow = false;
                } else if ( type == RowHeader ) {
                    rowHovered = cellIdx;
                    highlightRow = true;
                    highlightCol = false;
                }
            } else {
                highlightRow = false;
                highlightCol = false;
            }
        }
        ImGui::PopStyleColor( 3 );
    }
    static void CellProps( int targetId, int byte, float indentSpacing = 70 )
    {
        u16 const addr = 0x0000 + targetId;
        ImGui::BeginGroup();

        ImGui::Text( "Address:" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "$%04X", addr );
        ImGui::Unindent( indentSpacing );
        ImGui::Text( "Value:" );
        ImGui::SameLine();
        ImGui::Indent( indentSpacing );
        ImGui::Text( "$%02X", byte );
        ImGui::EndGroup();
    }
};
