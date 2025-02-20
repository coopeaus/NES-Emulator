#pragma once

#include <imgui.h>

// Forward declaration of Renderer.
class Renderer;

class UI
{
  public:
    // Rule of 5
    UI( const UI & ) = delete;
    UI( UI && ) = delete;
    UI &operator=( const UI & ) = delete;
    UI &operator=( UI && ) = delete;
    virtual ~UI() = default;

    UI( Renderer *renderer ) : renderer( renderer ) {}

    bool renderDebugWindows = true;
    bool showDemoWindow = true;

    Renderer *renderer;
    void      Render();
};
