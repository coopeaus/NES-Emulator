#pragma once

#include <imgui.h>

// Forward declaration of Renderer.
class Renderer;

class UI
{
  public:
    UI( Renderer *renderer ) : renderer( renderer ) {}

    bool renderDebugWindows = true;
    bool showDemoWindow = true;

    Renderer *renderer;
    void      Render();
};
