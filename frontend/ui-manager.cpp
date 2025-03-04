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
#include "register-viewer.h"
#include "pattern-tables.h"

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
    AddComponent<RegisterViewerWindow>( renderer );
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
