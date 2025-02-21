
// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the ImGuiWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of the main viewport + call
// BeginMenuBar() into it.

#include "renderer.h"
#include <imgui.h>
inline void mainMenuBar( Renderer *renderer, UI *ui ) // NOLINT
{
    if ( ImGui::BeginMainMenuBar() ) {
        if ( ImGui::BeginMenu( "File" ) ) {
            if ( ImGui::MenuItem( "Exit" ) ) {
                renderer->running = false;
            }
            ImGui::EndMenu();
        }
        if ( ImGui::BeginMenu( "View" ) ) {
            ImGui::MenuItem( "UI Demo", nullptr, &ui->showDemoWindow );
            ImGui::MenuItem( "Overlay", nullptr, &ui->showOverlay );

            // if ( ImGui::MenuItem( "Undo", "CTRL+Z" ) ) {
            // }
            // if ( ImGui::MenuItem( "Redo", "CTRL+Y", false, false ) ) {
            // } // Disabled item
            // ImGui::Separator();
            // if ( ImGui::MenuItem( "Cut", "CTRL+X" ) ) {
            // }
            // if ( ImGui::MenuItem( "Copy", "CTRL+C" ) ) {
            // }
            // if ( ImGui::MenuItem( "Paste", "CTRL+V" ) ) {
            // }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}
