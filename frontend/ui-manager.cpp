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

UIManager::UIManager( Renderer *renderer )
{
    AddComponent<MainMenuBar>( renderer );
    AddComponent<OverlayWindow>( renderer );
    AddComponent<DemoWindow>( renderer );
    AddComponent<DebuggerWindow>( renderer );
    AddComponent<LogWindow>( renderer );
    AddComponent<MemoryDisplayWindow>( renderer );
    AddComponent<PaletteWindow>( renderer );
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
