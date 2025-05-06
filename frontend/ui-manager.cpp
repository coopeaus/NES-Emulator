#define SDL_MAIN_HANDLED

#include "ui-manager.h"
#include "ui-component.h"

// UI components
#include "main-menubar.h"
#include "demo-window.h"
#include "overlay.h"
#include "debugger.h"
#include "log.h"
#include "memory-display.h"
#include "palettes.h"
#include "cpu-viewer.h"
#include "ppu-viewer.h"
#include "pattern-tables.h"
#include "sprites.h"
#include "nametable.h"
#include "cartridge-info.h"
#include "notification.h"
#include "input.h"

UIManager::UIManager( Renderer *renderer )
{
  AddComponent<MainMenuBar>( renderer );
  AddComponent<OverlayWindow>( renderer );
  AddComponent<DemoWindow>( renderer );
  AddComponent<DebuggerWindow>( renderer );
  AddComponent<LogWindow>( renderer );
  AddComponent<MemoryDisplayWindow>( renderer );
  AddComponent<PaletteWindow>( renderer );
  AddComponent<PatternTablesWindow>( renderer );
  AddComponent<SpritesWindow>( renderer );
  AddComponent<CpuViewerWindow>( renderer );
  AddComponent<PpuViewerWindow>( renderer );
  AddComponent<NametableWindow>( renderer );
  AddComponent<CartridgeInfoWindow>( renderer );
  AddComponent<Notification>( renderer );
  AddComponent<InputWindow>( renderer );
}

void UIManager::ToggleOverlay()
{
  if ( auto *win = GetComponent<OverlayWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleDebuggerWindow()
{
  if ( auto *win = GetComponent<DebuggerWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleMemory()
{
  if ( auto *win = GetComponent<MemoryDisplayWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleLog()
{
  if ( auto *win = GetComponent<LogWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleCpu()
{
  if ( auto *win = GetComponent<CpuViewerWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::TogglePpu()
{
  if ( auto *win = GetComponent<PpuViewerWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleCartridge()
{
  if ( auto *win = GetComponent<CartridgeInfoWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::TogglePalettes()
{
  if ( auto *win = GetComponent<PaletteWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::TogglePatternTables()
{
  if ( auto *win = GetComponent<PatternTablesWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleNametables()
{
  if ( auto *win = GetComponent<NametableWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleSprites()
{
  if ( auto *win = GetComponent<SpritesWindow>() ) {
    win->visible = !win->visible;
  }
}
void UIManager::ToggleInput()
{
  if ( auto *win = GetComponent<InputWindow>() ) {
    win->visible = !win->visible;
  }
}

void UIManager::Render()
{
  // Render all visible components
  for ( auto &comp : _components ) {
    if ( comp->visible ) {
      comp->OnVisible();
      comp->RenderSelf();
    } else {
      comp->OnHidden();
    }
  }
}
