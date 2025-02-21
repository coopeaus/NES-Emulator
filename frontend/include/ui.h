#pragma once

#include <imgui.h>

// Forward declaration of Renderer.
class Renderer;

class UI
{
  public:
    UI( Renderer *renderer );

    bool renderDebugWindows = true;
    bool showMainMenuBar = true;
    bool showOverlay = true;
    bool showDemoWindow = false;

    Renderer *renderer;
    void      Render();
};
