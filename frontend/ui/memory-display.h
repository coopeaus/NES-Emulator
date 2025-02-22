#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class MemoryDisplayWindow : public UIComponent
{
  public:
    MemoryDisplayWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void RenderSelf() override
    {
        ImGuiWindowFlags windowFlags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 600, -1 ), ImVec2( 800, -1 ) );

        if ( ImGui::Begin( "Memory Viewer", &visible, windowFlags ) ) {
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
            

            // Eventually we can add in PPU memory, etc. here
            const char *items[] = {
                "CPU Memory",
                "PPU Memory"
            };
            static int item_selected_idx = 0;

            const char* combo_preview = items[item_selected_idx];
            if (ImGui::BeginCombo("Memory Type", combo_preview)) {
                for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
                    const bool is_selected = (item_selected_idx == n);
                    if (ImGui::Selectable(items[n], is_selected)) {
                        item_selected_idx = n;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            if (item_selected_idx == 0) {
                ImGui::Text("CPU Memory:");
                ImGui::BeginChild("CPU Memory", ImVec2(0, 260), true, ImGuiWindowFlags_HorizontalScrollbar);
            
                // Use ImGui table to enforce column alignment
                if (ImGui::BeginTable("MemoryTable", 17, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 40);

                    // One column for each byte
                    for (int j = 0; j < 16; j++) {
                        char colName[4];
                        snprintf(colName, sizeof(colName), "%02X", j);
                        ImGui::TableSetupColumn(colName, ImGuiTableColumnFlags_WidthFixed, 20);
                    }
                    ImGui::TableHeadersRow();
            
                    // Memory display
                    for (int i = 0; i < 0x10000; i += 16) {
                        ImGui::TableNextRow();
                        
                        // Address column
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%04X", i);

                        // Hex byte columns
                        for (int j = 0; j < 16; j++) {
                            uint8_t byte = renderer->bus.cpu.Read(i + j);

                            ImGui::TableSetColumnIndex(j + 1);
                            ImGui::Text("%02X", byte);
                        }
                    }
            
                    ImGui::EndTable();
                }
            
                ImGui::EndChild();
            }
        }

        
        ImGui::End();
        ImGui::PopStyleVar();
    }

  private:
    bool _cycleBreakpoint = false;

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
};
