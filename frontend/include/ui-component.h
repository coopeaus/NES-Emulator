#pragma once

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

    virtual void OnVisible() = 0;
    virtual void OnHidden() = 0;
    virtual void RenderSelf() = 0;

    void Show() { visible = true; }
    void Hide() { visible = false; }

    bool visible{};

  protected:
    Renderer *renderer;
};
