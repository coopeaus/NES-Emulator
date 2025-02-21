#pragma once
#include <imgui.h>

class Renderer;

class UIComponent
{
  public:
    UIComponent( const UIComponent & ) = default;
    UIComponent( UIComponent && ) = delete;
    UIComponent &operator=( const UIComponent & ) = default;
    UIComponent &operator=( UIComponent && ) = delete;
    virtual ~UIComponent() = default;

    UIComponent( Renderer *renderer ) : renderer( renderer ) {}

    virtual void RenderSelf() = 0;

    void Show() { visible = true; }
    void Hide() { visible = false; }

    bool visible{};

  protected:
    Renderer *renderer;
};
