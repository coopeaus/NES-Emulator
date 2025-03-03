#pragma once
#include "theme.h"
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

class PatternTablesWindow : public UIComponent
{
  public:
    PatternTablesWindow( Renderer *renderer ) : UIComponent( renderer ) { visible = true; }

    /*
    ################################
    #           Variables          #
    ################################
    */
    enum TabType : int { Table0, Table1 };
    int tabSelected = Table0;
    int pattern0Selected = 0;

    /*
    ################################
    #            Methods           #
    ################################
    */

    void OnVisible() override { renderer->updatePatternTables = true; }
    void OnHidden() override { renderer->updatePatternTables = false; }
    void RenderSelf() override
    {
        ImGuiWindowFlags const windowFlags = ImGuiWindowFlags_MenuBar;
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
        ImGui::SetNextWindowSizeConstraints( ImVec2( 620, 450 ), ImVec2( 1000, 600 ) );

        if ( ImGui::Begin( "Tiles", &visible, windowFlags ) ) {
            RenderMenuBar();

            ImGui::PushFont( renderer->fontMono );
            LeftPanel();
            ImGui::SameLine();
            RightPanel();
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }
    void RenderTabs()
    {
        ImGuiTabBarFlags const tabBarFlags = ImGuiTabBarFlags_None;

        if ( ImGui::BeginTabBar( "PaletteTabs", tabBarFlags ) ) {

            if ( ImGui::BeginTabItem( "Table0" ) ) {
                tabSelected = Table0;
                ImGui::EndTabItem();
            }

            if ( ImGui::BeginTabItem( "Table1" ) ) {
                tabSelected = Table1;
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }

    void RenderGrid()
    {
        for ( int rowStart = 0; rowStart < 256; rowStart += 16 ) {
            ImGui::Dummy( ImVec2( 0, 0 ) );
            for ( int cell = 0; cell < 16; cell++ ) {
                ImGui::SameLine();
                bool isSelected = pattern0Selected == rowStart + cell;
                Cell( rowStart + cell, &isSelected );
            }
        }
    }

    void Cell( int id, bool *isSelected, float hw = 20.0, const char *label = nullptr )
    {
        ImGui::PushID( id );
        ImVec2 const  size = ImVec2( hw, hw );
        ImColor const transparent = ImColor( 0, 0, 0, 0 );

        if ( CustomTheme::selectableTransparent( isSelected, ImGuiSelectableFlags_None, size, label ) ) {
            pattern0Selected = id;
        }
        ImGui::PopID();
    }

    void LeftPanel()
    {
        ImGui::PushStyleColor( ImGuiCol_ChildBg, Spectrum::GRAY100 );
        ImGui::BeginChild( "left panel", ImVec2( 330, 330 ), ImGuiChildFlags_Borders );

        RenderTabs();
        int const tableIdx = tabSelected == Table0 ? 0 : 1;
        GLuint    textureHandle = renderer->GrabPatternTableTextureHandle( tableIdx );
        ImGui::Image( (ImTextureID) (intptr_t) textureHandle, ImVec2( 320, 320 ) );

        // Offset interactive grid position to render over the top of the image
        ImVec2 cursorPos = ImGui::GetCursorPos();
        float  offsetY = -325.0f;
        float  offsetX = -5.0f;
        ImGui::SetCursorPos( ImVec2( cursorPos.x + offsetX, cursorPos.y + offsetY ) );
        RenderGrid();
        ImGui::SetCursorPos( cursorPos );
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void RightPanel()
    {
        ImGui::BeginChild( "right panel", ImVec2( 0, 0 ) );

        ImGui::PushFont( renderer->fontMonoBold );
        ImGui::Text( "Properties" );
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Text( "Right Panel" );
        ImGui::EndChild();
    }

    void RenderMenuBar()
    {
        if ( ImGui::BeginMenuBar() ) {
            if ( ImGui::BeginMenu( "File" ) ) {
                if ( ImGui::MenuItem( "Close" ) ) {
                    visible = false;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }
};
