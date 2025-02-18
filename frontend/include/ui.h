#pragma once

#include <imgui.h>

class UI
{
  public:
    virtual ~UI() = default;
    UI();

    // Rule of 5
    UI( const UI & ) = delete;            // No copy constructor
    UI( UI && ) = delete;                 // No move constructor
    UI &operator=( const UI & ) = delete; // No copy assignment
    UI &operator=( UI && ) = delete;      // No move assignment

    void         Init();
    virtual void Update();
    void         Render();
    void         Shutdown();
};
