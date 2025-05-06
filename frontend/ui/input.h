#pragma once
#include "ui-component.h"
#include "renderer.h"
#include <imgui.h>

using InputIdx = Renderer::InputIndex;

class InputWindow : public UIComponent
{
public:
  InputWindow( Renderer *renderer )
      : UIComponent( renderer ), bindingCurrentKeyboardKey( renderer->bindingCurrentKeyboardKey ),
        bindingCurrentGamepadKey( renderer->bindingCurrentGamepadKey ), keyboardIdx( renderer->keyboardIdx ),
        gamepad1Idx( renderer->gamepad1Idx ), gamepad2Idx( renderer->gamepad2Idx )
  {
    visible = false;
  }

  SDL_Scancode             &bindingCurrentKeyboardKey;
  SDL_GameControllerButton &bindingCurrentGamepadKey;
  u8                       &keyboardIdx;
  u8                       &gamepad1Idx;
  u8                       &gamepad2Idx;

  // internal state to help with selecting
  u8 selectedKeyboardRow = InputIdx::INPUT_NONE;
  u8 selectedGamepad1Row = InputIdx::INPUT_NONE;
  u8 selectedGamepad2Row = InputIdx::INPUT_NONE;

  /*
  ################################
  #           Variables          #
  ################################
  */
  enum TabType : u8 { KEYBOARD, GAMEPAD1, GAMEPAD2 };
  int tabSelected = KEYBOARD;

  enum BindType : u8 {
    BIND_KEYBOARD,
    BIND_GAMEPAD1,
    BIND_GAMEPAD2,
  };

  /*
  ################################
  #            Methods           #
  ################################
  */
  void OnVisible() override {}
  void OnHidden() override {}

  void RenderSelf() override
  {
    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |
                                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImVec2 const windowSize = ImVec2( 350, 370 );
    ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 10.0f, 10.0f ) );
    ImGui::SetNextWindowSizeConstraints( windowSize, windowSize );
    ImGui::PopStyleVar();

    ImGui::Begin( "Input Settings", &visible, windowFlags );
    RenderMenuBar();
    SettingsWindow();
    if ( ImGui::Button( "Reset to defaults" ) ) {
      ClearBindingState();
      renderer->ResetDefaultBindings();
    }
    ImGui::SameLine();
    if ( ImGui::Button( "Close" ) ) {
      ClearBindingState();
      visible = false;
    }
    ImGui::End();
  }

  void SettingsWindow()
  {
    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImGui::Spectrum::GRAY100 );
    ImGui::BeginChild( "input settings window", ImVec2( 330, 260 ), ImGuiChildFlags_Borders );
    ImGuiTabBarFlags const tabBarFlags = ImGuiTabBarFlags_None;
    if ( ImGui::BeginTabBar( "PaletteTabs", tabBarFlags ) ) {

      if ( ImGui::BeginTabItem( "Keyboard" ) ) {
        tabSelected = KEYBOARD;
        ImGui::Text( "Keyboard Input" );
        Keybinds( "Bind Keyboard Input", ImVec2( 310, 0 ), 2 );
        ImGui::EndTabItem();
      }

      if ( ImGui::BeginTabItem( "Controller 1" ) ) {
        ImGui::Text( "Gamepad1 Input" );
        tabSelected = GAMEPAD1;
        Keybinds( "Bind Controller1 Input", ImVec2( 310, 0 ), 2 );
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();
  }

  void Keybinds( const char *label, ImVec2 size, int columns )
  {
    static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_SizingStretchSame |
                                   ImGuiTableFlags_Borders | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX;
    ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    ImGui::BeginChild( label, size, ImGuiChildFlags_Border );
    ImGui::PopStyleColor();
    ImGui::BeginTable( label, columns, flags );

    // Hug the table border style
    ImGui::PushStyleVarY( ImGuiStyleVar_CellPadding, 0.0f );
    ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

    auto row = [this]( InputIdx btn ) {
      if ( tabSelected == KEYBOARD )
        ConfigKeyboardRow( btn );
      else if ( tabSelected == GAMEPAD1 )
        ConfigGamepad1Row( btn );
      else if ( tabSelected == GAMEPAD2 )
        ConfigGamepad2Row( btn, 10 );
    };

    row( InputIdx::A );
    row( InputIdx::B );
    row( InputIdx::SELECT );
    row( InputIdx::START );
    row( InputIdx::UP );
    row( InputIdx::DOWN );
    row( InputIdx::LEFT );
    row( InputIdx::RIGHT );

    ImGui::PopStyleVar( 2 );
    ImGui::EndTable();
    ImGui::EndChild();
  }

  void ClearBindingState()
  {
    selectedKeyboardRow = InputIdx::INPUT_NONE;
    selectedGamepad1Row = InputIdx::INPUT_NONE;
    selectedGamepad2Row = InputIdx::INPUT_NONE;
    keyboardIdx = InputIdx::INPUT_NONE;
    gamepad1Idx = InputIdx::INPUT_NONE;
    gamepad2Idx = InputIdx::INPUT_NONE;
    renderer->isBindingKeyboard = false;
    renderer->isBindingGamepad1 = false;
    renderer->isBindingGamepad2 = false;
    bindingCurrentKeyboardKey = SDL_SCANCODE_UNKNOWN;
    bindingCurrentGamepadKey = SDL_CONTROLLER_BUTTON_INVALID;
  }

  bool IsBindingDone( BindType type )
  {
    switch ( type ) {
      case BIND_KEYBOARD: return !renderer->isBindingKeyboard;
      case BIND_GAMEPAD1: return !renderer->isBindingGamepad1;
      case BIND_GAMEPAD2: return !renderer->isBindingGamepad2;
      default           : return false;
    }
  }

  void SetBindingState( BindType type, InputIdx target )
  {
    switch ( type ) {
      case BIND_KEYBOARD:
        keyboardIdx = target;
        gamepad1Idx = InputIdx::INPUT_NONE;
        gamepad2Idx = InputIdx::INPUT_NONE;
        selectedKeyboardRow = target;
        selectedGamepad1Row = InputIdx::INPUT_NONE;
        selectedGamepad2Row = InputIdx::INPUT_NONE;
        renderer->isBindingKeyboard = true;
        bindingCurrentKeyboardKey = renderer->keyboardBinds.at( target );
        break;
      case BIND_GAMEPAD1:
        keyboardIdx = InputIdx::INPUT_NONE;
        gamepad1Idx = target;
        gamepad2Idx = InputIdx::INPUT_NONE;
        selectedKeyboardRow = InputIdx::INPUT_NONE;
        selectedGamepad1Row = target;
        selectedGamepad2Row = InputIdx::INPUT_NONE;
        renderer->isBindingGamepad1 = true;
        bindingCurrentGamepadKey = renderer->gamepad1Binds.at( target );
        break;
      case BIND_GAMEPAD2:
        keyboardIdx = InputIdx::INPUT_NONE;
        gamepad1Idx = InputIdx::INPUT_NONE;
        gamepad2Idx = target;
        selectedKeyboardRow = InputIdx::INPUT_NONE;
        selectedGamepad1Row = InputIdx::INPUT_NONE;
        selectedGamepad2Row = target;
        renderer->isBindingGamepad2 = true;
        bindingCurrentGamepadKey = renderer->gamepad2Binds.at( target );
        break;
      default:
    }
  }

  std::string GetBindBtnStr( BindType type, InputIdx target )
  {
    switch ( type ) {
      case BIND_KEYBOARD: return GetBindedKeyboardStr( target ); break;
      case BIND_GAMEPAD1:
      case BIND_GAMEPAD2: return GetBindedGamepadStr( target ); break;
      default           : return "";
    }
  }

  static void RowStart( InputIdx btn )
  {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex( 0 );
    ImGui::PushStyleVar( ImGuiStyleVar_SelectableTextAlign, ImVec2( 0.5f, 0.5f ) );
    TextCenter( GetGamepadBtnStr( btn ) );
    ImGui::PopStyleVar();
    ImGui::TableSetColumnIndex( 1 );
  }

  static void RowEnd() { ImGui::EndDisabled(); }

  void RowConfig( InputIdx &target, u8 &selectedId, BindType type, int idxOffset = 0 )
  {
    RowStart( target );
    ImGui::PushID( target + idxOffset );
    bool isDisabled = ( selectedId != InputIdx::INPUT_NONE ) && ( selectedId != target );
    ImGui::BeginDisabled( isDisabled );
    if ( selectedId == target ) {
      ImGui::Button( "Waiting for input...", ImVec2( -FLT_MIN, 0.0f ) );
      if ( IsBindingDone( type ) ) {
        ClearBindingState();
      }
    } else {
      if ( ImGui::Button( GetBindBtnStr( type, target ).c_str(), ImVec2( -FLT_MIN, 0.0f ) ) ) {
        SetBindingState( type, target );
      }
    }
    ImGui::PopID();
    RowEnd();
  }

  void ConfigKeyboardRow( InputIdx btn, int idxOffset = 0 )
  {
    RowConfig( btn, selectedKeyboardRow, BIND_KEYBOARD, idxOffset );
  }

  void ConfigGamepad1Row( InputIdx btn, int idxOffset = 10 )
  {
    RowConfig( btn, selectedGamepad1Row, BIND_GAMEPAD1, idxOffset );
  }

  void ConfigGamepad2Row( InputIdx btn, int idxOffset = 20 )
  {
    RowConfig( btn, selectedGamepad2Row, BIND_GAMEPAD2, idxOffset );
  }

  static const char *GetGamepadBtnStr( InputIdx btn )
  {
    switch ( btn ) {
      case InputIdx::A         : return "A";
      case InputIdx::B         : return "B";
      case InputIdx::SELECT    : return "SELECT";
      case InputIdx::START     : return "START";
      case InputIdx::UP        : return "UP";
      case InputIdx::DOWN      : return "DOWN";
      case InputIdx::LEFT      : return "LEFT";
      case InputIdx::RIGHT     : return "RIGHT";
      case InputIdx::INPUT_NONE: return "";
    }
  }

  const char *GetBindedKeyboardStr( InputIdx btn )
  {
    auto current = renderer->keyboardBinds[btn];
    if ( current == SDL_SCANCODE_UNKNOWN ) {
      return "";
    }
    return Renderer::GetScancodeName( (SDL_Scancode) current );
  }

  const char *GetBindedGamepadStr( InputIdx btn )
  {
    auto current = renderer->gamepad1Binds[btn];
    if ( current == SDL_CONTROLLER_BUTTON_INVALID ) {
      return "";
    }
    return Renderer::GetControllerKeyName( (SDL_GameControllerButton) current );
  }

  static void TextCenter( const char *text, ... )
  {
    va_list vaList = nullptr;
    va_start( vaList, text );
    float fontSize = ImGui::GetFontSize() * (float) strlen( text ) / 2;
    ImGui::SameLine( ( ImGui::GetWindowSize().x / 4 ) - fontSize + ( fontSize / 2 ) );
    ImGui::TextV( text, vaList );
    va_end( vaList );
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
