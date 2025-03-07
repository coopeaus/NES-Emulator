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
#include "nametable.h"

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
    AddComponent<CpuViewerWindow>( renderer );
    AddComponent<PpuViewerWindow>( renderer );
    AddComponent<NametableWindow>( renderer );
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
