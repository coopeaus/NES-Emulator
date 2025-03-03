#pragma once
#include "ui-component.h"
#include "renderer.h"
#include "ui-manager.h"
#include <imgui.h>

#include "demo-window.h"
#include "debugger.h"
#include "overlay.h"
#include "log.h"
#include "memory-display.h"
#include "palettes.h"
#include "pattern-tables.h"

class MainMenuBar : public UIComponent
{
  public:
    UIManager *ui;

    MainMenuBar( Renderer *renderer ) : UIComponent( renderer ), ui( &renderer->ui ) { visible = true; }

    void OnVisible() override {}
    void OnHidden() override {}

    void RenderSelf() override // NOLINT
    {
        if ( ImGui::BeginMainMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Exit" ) ) {
                    renderer->running = false;
                }
                ImGui::EndMenu();
            }
            if ( ImGui::BeginMenu( "Debug" ) ) {

                if ( auto *demoWindow = ui->GetComponent<DemoWindow>() ) {
                    ImGui::MenuItem( "UI Demo", nullptr, &demoWindow->visible );
                }

                if ( auto *debuggerWindow = ui->GetComponent<DebuggerWindow>() ) {
                    ImGui::MenuItem( "Debugger", nullptr, &debuggerWindow->visible );
                }

                if ( auto *logWindow = ui->GetComponent<LogWindow>() ) {
                    ImGui::MenuItem( "Trace Log", nullptr, &logWindow->visible );
                }

                if ( auto *memoryDisplayWindow = ui->GetComponent<MemoryDisplayWindow>() ) {
                    ImGui::MenuItem( "Memory Display", nullptr, &memoryDisplayWindow->visible );
                }

                if ( auto *paletteWindow = ui->GetComponent<PaletteWindow>() ) {
                    ImGui::MenuItem( "Palettes", nullptr, &paletteWindow->visible );
                }

                if ( auto *patternTablesWindow = ui->GetComponent<PatternTablesWindow>() ) {
                    ImGui::MenuItem( "Pattern Tables", nullptr, &patternTablesWindow->visible );
                }

                ImGui::EndMenu();
            }
        }
        if ( ImGui::BeginMenu( "Overlay" ) ) {

            if ( auto *overlayWindow = ui->GetComponent<OverlayWindow>() ) {
                ImGui::MenuItem( "Enabled", nullptr, &overlayWindow->visible );
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
};
