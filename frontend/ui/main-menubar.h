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
#include "cpu-viewer.h"
#include "ppu-viewer.h"
#include "pattern-tables.h"
#include "sprites.h"
#include "nametable.h"
#include "cartridge-info.h"

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
        if ( ImGui::MenuItem( "Open" ) ) {
          renderer->OpenRomFileDialog();
        }

        auto &recentRoms = renderer->recentRoms;
        bool  recentRomsEmpty = recentRoms.empty();
        if ( recentRomsEmpty ) {
          ImGui::TextDisabled( "Recent Files" );
        } else if ( ImGui::BeginMenu( "Recent Files" ) ) {
          for ( auto &romPath : recentRoms ) {
            if ( ImGui::MenuItem( romPath.c_str() ) ) {
              renderer->LoadNewCartridge( romPath );
              renderer->AddToRecentROMs( romPath );
            }
          }
          ImGui::Separator();
          // button to clear recent roms
          if ( ImGui::MenuItem( "Clear" ) ) {
            renderer->ClearRecentROMs();
          }
          ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu( "Test Roms" ) ) {
          auto testRoms = renderer->testRoms;
          for ( auto &romPath : testRoms ) {
            if ( ImGui::MenuItem( romPath.c_str() ) ) {
              renderer->LoadNewCartridge( romPath );
            }
          }
          ImGui::EndMenu();
        }
        ImGui::Separator();
        if ( ImGui::MenuItem( "Exit" ) ) {
          renderer->running = false;
        }
        ImGui::EndMenu();
      }
      if ( ImGui::BeginMenu( "Debug" ) ) {

        if ( auto *debuggerWindow = ui->GetComponent<DebuggerWindow>() ) {
          ImGui::MenuItem( "Debugger", nullptr, &debuggerWindow->visible );
        }

        if ( auto *memoryDisplayWindow = ui->GetComponent<MemoryDisplayWindow>() ) {
          ImGui::MenuItem( "Memory Display", nullptr, &memoryDisplayWindow->visible );
        }

        if ( auto *logWindow = ui->GetComponent<LogWindow>() ) {
          ImGui::MenuItem( "Trace Log", nullptr, &logWindow->visible );
        }
        ImGui::Separator();
        if ( auto *cpuViewer = ui->GetComponent<CpuViewerWindow>() ) {
          ImGui::MenuItem( "CPU", nullptr, &cpuViewer->visible );
        }

        if ( auto *registerViewer = ui->GetComponent<PpuViewerWindow>() ) {
          ImGui::MenuItem( "PPU", nullptr, &registerViewer->visible );
        }

        if ( auto *cartridgeInfoViewer = ui->GetComponent<CartridgeInfoWindow>() ) {
          ImGui::MenuItem( "Cartridge Info", nullptr, &cartridgeInfoViewer->visible );
        }
        ImGui::Separator();

        if ( auto *paletteWindow = ui->GetComponent<PaletteWindow>() ) {
          ImGui::MenuItem( "Palettes", nullptr, &paletteWindow->visible );
        }

        if ( auto *patternTablesWindow = ui->GetComponent<PatternTablesWindow>() ) {
          ImGui::MenuItem( "Pattern Tables", nullptr, &patternTablesWindow->visible );
        }

        if ( auto *nametableWindow = ui->GetComponent<NametableWindow>() ) {
          ImGui::MenuItem( "Nametables", nullptr, &nametableWindow->visible );
        }

        if ( auto *spritesWindow = ui->GetComponent<SpritesWindow>() ) {
          ImGui::MenuItem( "Sprites", nullptr, &spritesWindow->visible );
        }

        ImGui::EndMenu();
      }
      if ( ImGui::BeginMenu( "Overlay" ) ) {

        if ( auto *overlayWindow = ui->GetComponent<OverlayWindow>() ) {
          ImGui::MenuItem( "Enabled", nullptr, &overlayWindow->visible );
        }
        ImGui::EndMenu();
      }
      if ( ImGui::BeginMenu( "ImGui Demo" ) ) {

        if ( auto *demoWindow = ui->GetComponent<DemoWindow>() ) {
          ImGui::MenuItem( "Show", nullptr, &demoWindow->visible );
        }
        ImGui::EndMenu();
      }

      // save State Button
      if ( ImGui::BeginMenu( "States" ) ) {
        if ( ImGui::MenuItem( "Save Slot 0" ) ) {
          renderer->bus.QuickSaveState( 0 );
        }
        if ( ImGui::MenuItem( "Save Slot 1" ) ) {
          renderer->bus.QuickSaveState( 1 );
        }
        if ( ImGui::MenuItem( "Save Slot 2" ) ) {
          renderer->bus.QuickSaveState( 2 );
        }
        if ( ImGui::MenuItem( "Save Slot 3" ) ) {
          renderer->bus.QuickSaveState( 2 );
        }

        auto exists = [&]( int idx ) { return renderer->bus.DoesSaveSlotExist( idx ); };
        ImGui::BeginDisabled( !exists( 0 ) );
        if ( ImGui::MenuItem( "Load Slot 0" ) ) {
          renderer->bus.QuickLoadState( 0 );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 1 ) );
        if ( ImGui::MenuItem( "Load Slot 1" ) ) {
          renderer->bus.QuickLoadState( 1 );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 2 ) );
        if ( ImGui::MenuItem( "Load Slot 2" ) ) {
          renderer->bus.QuickLoadState( 2 );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 3 ) );
        if ( ImGui::MenuItem( "Load Slot 3" ) ) {
          renderer->bus.QuickLoadState( 3 );
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        if ( ImGui::MenuItem( "Save as" ) ) {
          renderer->SaveCurrentStateFileDialog();
        }
        if ( ImGui::MenuItem( "Load from file" ) ) {
          if ( !renderer->LoadStateFileDialog() ) {
            fmt::print( "Failed to load state\n" );
          }
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    };
  }
};
