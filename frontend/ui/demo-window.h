#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class DemoWindow : public UIComponent
{
  public:
    DemoWindow( Renderer *renderer ) : UIComponent( renderer )
    {
        // Default to hidden
        visible = false;
    }

    void RenderSelf() override { ImGui::ShowDemoWindow( &visible ); }
};
