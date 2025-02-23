#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class DemoWindow : public UIComponent
{
  public:
    DemoWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = false; }

    void OnVisible() override {}
    void OnHidden() override {}
    void RenderSelf() override { ImGui::ShowDemoWindow( &visible ); }
};
