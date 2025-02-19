#include "ui.h"
#include "renderer.h" // Now we have the complete definition of Renderer

void UI::Render()
{
    if ( !renderDebugWindows ) {
        return;
    }
    // Render ImGui overlay if active.
    if ( showDemoWindow ) {
        ImGui::ShowDemoWindow( &showDemoWindow );
    }
    // NOLINTBEGIN
    if ( showAnotherWindow ) {
        static float f = 0.0f;
        static int   counter = 0;

        ImGui::Begin( "Hello, world!", &showAnotherWindow );
        ImGui::Text( "This is some useful text." );
        ImGui::Checkbox( "Demo Window", &showDemoWindow );
        ImGui::Checkbox( "Another Window", &showAnotherWindow );
        ImGui::SliderFloat( "float", &f, 0.0f, 1.0f );
        ImGui::ColorEdit3( "clear color", (float *) &renderer->clearColor );
        if ( ImGui::Button( "Button" ) ) {
            counter++;
        }
        ImGui::SameLine();
        ImGui::Text( "counter = %d", counter );
        ImGui::Text( "CPU Cycle: %lld", renderer->bus.cpu.GetCycles() );
        ImGui::Text( "Application average %.3f ms/frame (%.1f FPS)", 1000.0f / renderer->io->Framerate,
                     renderer->io->Framerate );
        ImGui::End();
    }
    // NOLINTEND
}
