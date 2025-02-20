// Demo window

#include "renderer.h"
inline void demoWindow( Renderer *renderer, bool *showWindow )
{
    // NOLINTBEGIN
    static float f = 0.0f;
    static int   counter = 0;

    ImGui::Begin( "Hello, world!", showWindow );
    ImGui::Text( "This is some useful text." );
    ImGui::Checkbox( "Demo Window", showWindow );
    ImGui::Checkbox( "Another Window", showWindow );
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
    // NOLINTEND
}
