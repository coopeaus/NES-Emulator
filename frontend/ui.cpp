#include "ui.h"
#include "renderer.h"

// UI Windows
#include "demo-window.h"

void UI::Render()
{
    if ( !renderDebugWindows ) {
        return;
    }

    if ( showDemoWindow ) {
        demoWindow( renderer, &showDemoWindow );
    }
}
