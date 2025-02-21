#include "ui.h"
#include "renderer.h"

// UI
#include "main-menubar.h"
#include "demo-window.h"
#include "overlay.h"

UI::UI( Renderer *renderer ) : renderer( renderer )
{
    // UI init settings can go here, stuff that shouldn't run in the main render loop
}

void UI::Render()
{
    if ( !renderDebugWindows ) {
        return;
    }

    if ( showMainMenuBar ) {
        mainMenuBar( renderer, this );
    }

    if ( showOverlay ) {
        overlay( renderer, this );
    }

    if ( showDemoWindow ) {
        demoWindow( renderer, &showDemoWindow );
    }
}
