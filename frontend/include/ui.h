#pragma once

#include <SDL_stdinc.h>
#include <imgui.h>

// Forward declaration of Renderer.
class Renderer;

class UI
{
  public:
    bool renderDebugWindows = true;
    bool showDemoWindow = true;
    bool showAnotherWindow = true;

    Renderer *renderer;
    UI( Renderer *renderer ) : renderer( renderer ) {}

    void Render();
};
