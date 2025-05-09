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
#include "input.h"

#if defined( __APPLE__ )
#define CMD "Cmd"
#else
#define CMD "Ctrl"
#endif

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
        if ( ImGui::MenuItem( "Open", CMD "+O" ) ) {
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
      if ( ImGui::BeginMenu( "Game" ) ) {
        if ( ImGui::MenuItem( "Pause", "Esc", renderer->paused ) ) {
          renderer->PauseToggle();
          renderer->NotifyStart( renderer->paused ? "Paused" : "Unpaused" );
        }
        if ( ImGui::MenuItem( "Debug Reset", CMD "+R" ) ) {
          renderer->bus.DebugReset();
          renderer->NotifyStart( "Debug Reset" );
        }
        ImGui::Separator();
        if ( ImGui::MenuItem( "Hardware Reset" ) ) {
          renderer->bus.PowerCycle();
          renderer->NotifyStart( "Hardware Reset" );
        }

        ImGui::EndMenu();
      }

      // save State Button
      if ( ImGui::BeginMenu( "State" ) ) {
        if ( ImGui::MenuItem( "Save Slot 0", CMD "+S" ) ) {
          renderer->bus.QuickSaveState( 0 );
          renderer->NotifyStart( "Saved to slot 0." );
        }
        if ( ImGui::MenuItem( "Save Slot 1", "Numpad 1" ) ) {
          renderer->bus.QuickSaveState( 1 );
          renderer->NotifyStart( "Saved to slot 1." );
        }
        if ( ImGui::MenuItem( "Save Slot 2", "Numpad 2" ) ) {
          renderer->bus.QuickSaveState( 2 );
          renderer->NotifyStart( "Saved to slot 2." );
        }
        if ( ImGui::MenuItem( "Save Slot 3", "Numpad 3" ) ) {
          renderer->bus.QuickSaveState( 2 );
          renderer->NotifyStart( "Saved to slot 3." );
        }

        auto exists = [&]( int idx ) { return renderer->bus.DoesSaveSlotExist( idx ); };
        ImGui::BeginDisabled( !exists( 0 ) );
        if ( ImGui::MenuItem( "Load Slot 0", CMD "+L" ) ) {
          renderer->bus.QuickLoadState( 0 );
          renderer->NotifyStart( "Loaded from slot 0." );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 1 ) );
        if ( ImGui::MenuItem( "Load Slot 1", CMD "+Numpad 1" ) ) {
          renderer->bus.QuickLoadState( 1 );
          renderer->NotifyStart( "Loaded from slot 1." );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 2 ) );
        if ( ImGui::MenuItem( "Load Slot 2", CMD "+Numpad 2" ) ) {
          renderer->bus.QuickLoadState( 2 );
          renderer->NotifyStart( "Loaded from slot 2." );
        }
        ImGui::EndDisabled();

        ImGui::BeginDisabled( !exists( 3 ) );
        if ( ImGui::MenuItem( "Load Slot 3", CMD "+Numpad 3" ) ) {
          renderer->bus.QuickLoadState( 3 );
          renderer->NotifyStart( "Loaded from slot 3." );
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        if ( ImGui::MenuItem( "Save as", CMD "+Shift+S" ) ) {
          renderer->SaveCurrentStateFileDialog();
        }
        if ( ImGui::MenuItem( "Load from file", CMD "+Shift+L" ) ) {
          if ( !renderer->LoadStateFileDialog() ) {
            fmt::print( "Failed to load state\n" );
          }
        }
        ImGui::EndMenu();
      }

      if ( ImGui::BeginMenu( "Controls" ) ) {
        if ( auto *input = ui->GetComponent<InputWindow>() ) {
          ImGui::MenuItem( "Input Settings", CMD "+Shift+I", &input->visible );
        }
        ImGui::EndMenu();
      }
      if ( ImGui::BeginMenu( "Debug" ) ) {

        if ( auto *overlayWindow = ui->GetComponent<OverlayWindow>() ) {
          ImGui::MenuItem( "Overlay", "F1", &overlayWindow->visible );
        }

        if ( auto *cartridgeInfoViewer = ui->GetComponent<CartridgeInfoWindow>() ) {
          ImGui::MenuItem( "Cartridge Info", "F2", &cartridgeInfoViewer->visible );
        }

        ImGui::Separator();

        if ( auto *debuggerWindow = ui->GetComponent<DebuggerWindow>() ) {
          ImGui::MenuItem( "Debugger", CMD "+Shift+D", &debuggerWindow->visible );
        }

        if ( auto *memoryDisplayWindow = ui->GetComponent<MemoryDisplayWindow>() ) {
          ImGui::MenuItem( "Memory Display", CMD "+Shift+M", &memoryDisplayWindow->visible );
        }

        if ( auto *logWindow = ui->GetComponent<LogWindow>() ) {
          ImGui::MenuItem( "Trace Log", CMD "+Shift+T", &logWindow->visible );
        }
        ImGui::Separator();
        if ( auto *cpuViewer = ui->GetComponent<CpuViewerWindow>() ) {
          ImGui::MenuItem( "CPU", CMD "+Shift+C", &cpuViewer->visible );
        }

        if ( auto *registerViewer = ui->GetComponent<PpuViewerWindow>() ) {
          ImGui::MenuItem( "PPU", CMD "+Shift+P", &registerViewer->visible );
        }

        ImGui::Separator();

        if ( auto *paletteWindow = ui->GetComponent<PaletteWindow>() ) {
          ImGui::MenuItem( "Palettes", CMD "+1", &paletteWindow->visible );
        }

        if ( auto *patternTablesWindow = ui->GetComponent<PatternTablesWindow>() ) {
          ImGui::MenuItem( "Pattern Tables", CMD "+2", &patternTablesWindow->visible );
        }

        if ( auto *nametableWindow = ui->GetComponent<NametableWindow>() ) {
          ImGui::MenuItem( "Nametables", CMD "+3", &nametableWindow->visible );
        }

        if ( auto *spritesWindow = ui->GetComponent<SpritesWindow>() ) {
          ImGui::MenuItem( "Sprites", CMD "+4", &spritesWindow->visible );
        }

        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    };
  }
};
