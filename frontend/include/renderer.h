#pragma once

// Graphics
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_gamecontroller.h>
#include <SDL_hints.h>
#include <SDL2/SDL.h>
#include <SDL_video.h>
#include <imgui.h>

// Libraries
#include <algorithm>
#include <array>
#include <cstdlib>
#include <glad/glad.h>
#include <csignal>
#include <numeric>
#include <string>
#include <thread>
#include <cstdint>
#include <iostream>
#include <deque>
#include <filesystem>
#include <fstream>
#include <chrono>
#include "tinyfiledialogs.h"

// Ours
#include "theme.h"
#include "bus.h"
#include "cartridge.h"
#include "ui-component.h"
#include "ui-manager.h"
#include "paths.h"
#include "Sound_Queue.h"

using u32 = uint32_t;
using u64 = uint64_t;

class UI;
class Renderer
{
public:
  /*
  ################################
  #          window info         #
  ################################
  */
  int         nesWidth = 256;
  int         nesHeight = 240;
  int         windowWidth = nesWidth * 2;
  int         windowHeight = nesHeight * 2;
  int         bufferSize = nesWidth * nesHeight;
  std::string windowTitle = "NES Emulator";

  // Control settings
  std::array<SDL_Scancode, 8>             keyboardBinds{ SDL_SCANCODE_X,      SDL_SCANCODE_Z,    SDL_SCANCODE_TAB,
                                             SDL_SCANCODE_RETURN, SDL_SCANCODE_UP,   SDL_SCANCODE_DOWN,
                                             SDL_SCANCODE_LEFT,   SDL_SCANCODE_RIGHT };
  std::array<SDL_GameControllerButton, 8> gamepad1Binds{
      SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
      SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
      SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
  std::array<SDL_GameControllerButton, 8> gamepad2Binds{
      SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
      SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
      SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
  SDL_GameController *gamepad1{};
  SDL_GameController *gamepad2{};

  bool                     isBindingKeyboard = false;
  bool                     test = false;
  bool                     isBindingGamepad1 = false;
  bool                     isBindingGamepad2 = false;
  SDL_Scancode             bindingCurrentKeyboardKey = SDL_SCANCODE_UNKNOWN;
  SDL_GameControllerButton bindingCurrentGamepadKey = SDL_CONTROLLER_BUTTON_INVALID;

  enum InputIndex : u8 { A, B, SELECT, START, UP, DOWN, LEFT, RIGHT, INPUT_NONE };
  u8 keyboardIdx = INPUT_NONE;
  u8 gamepad1Idx = INPUT_NONE;
  u8 gamepad2Idx = INPUT_NONE;

  static const char *GetScancodeName( SDL_Scancode scancode ) { return SDL_GetScancodeName( scancode ); }
  static const char *GetControllerKeyName( SDL_GameControllerButton code )
  {
    return SDL_GameControllerGetStringForButton( code );
  }
  void PrintKeyboardBinds()
  {
    for ( auto &bind : keyboardBinds ) {
      const auto *name = GetScancodeName( bind );
      fmt::print( "Key: {}\n", name );
    }
  }

  /*
  ################################
  #       Render Variables       #
  ################################
  */
  SDL_Window   *window = nullptr;
  SDL_GLContext glContext = nullptr;
  ImGuiIO      *io{};
  ImVec4        clearColor = ImVec4( 0.00F, 0.00F, 0.00F, 1.00F );
  ImFont       *fontMenu = nullptr;
  ImFont       *fontMono = nullptr;
  ImFont       *fontMonoBold = nullptr;
  GLuint        emuScreenTexture = 0;
  GLuint        emuScreenShaderProgram = 0;
  GLuint        emuScreenVAO = 0;
  GLuint        emuScreenVBO = 0;

  // frame clock
  using Clock = std::chrono::steady_clock;
  Clock::time_point frameStart;

  // Textures
  GLuint patternTable0Texture = 0;
  GLuint patternTable1Texture = 0;
  GLuint nametable0Texture = 0;
  GLuint nametable1Texture = 0;
  GLuint nametable2Texture = 0;
  GLuint nametable3Texture = 0;
  GLuint oamTexture = 0;

  /*
  ################################
  ||       Audio Variables      ||
  ################################
  */
  std::unique_ptr<Sound_Queue> soundQueue;
  static int const             audioBufferSize = 2048;
  blip_sample_t                audioBuffer[audioBufferSize]{};

  /*
  ################################
  #       global variables       #
  ################################
  */

  // notification message
  bool              messageShow = false;
  std::string       message;
  int               messageDuration = 3;
  Clock::time_point messageStart;

  bool running = true;
  bool paused = false;
  void PauseToggle() { paused = !paused; }

  bool updatePatternTables = false;
  bool updateNametables = false;
  bool updateOam = false;

  u64                    currentFrame = 0;
  std::array<u32, 16384> patternTable0Buffer{};
  std::array<u32, 16384> patternTable1Buffer{};
  std::array<u32, 61440> nametable0Buffer{};
  std::array<u32, 61440> nametable1Buffer{};
  std::array<u32, 61440> nametable2Buffer{};
  std::array<u32, 61440> nametable3Buffer{};
  std::array<u32, 4096>  oamBuffer{};

  // Sampling metrics
  std::vector<double>            frameTimes;
  std::vector<double>            cpuCycles;
  u64                            lastSampleCycles = 0;
  Clock::time_point              lastSampleTime = Clock::now();
  std::chrono::time_point<Clock> lastFrameTime = Clock::now();

#define ROM( x ) ( std::string( paths::roms() ) + "/" + ( x ) )
  std::vector<std::string> testRoms = {

      ROM( "custom.nes" ),  ROM( "color_test.nes" ), ROM( "nestest.nes" ),
      ROM( "palette.nes" ), ROM( "scanline.nes" ),   ROM( "instr_test-v5.nes" ),
  };
  std::deque<std::string> recentRoms;
  std::string             recentRomDir;
  std::string             recentStatefileDir;
  enum RomSelected : u8 { CUSTOM, COLOR_TEST, NESTEST, PALETTE, SCANLINE, V5 };
  u8 romSelected = RomSelected::CUSTOM;

  /*
  ################################
  #          peripherals         #
  ################################
  */
  UIManager   ui;
  Bus         bus;
  CPU        &cpu = bus.cpu;
  PPU        &ppu = bus.ppu;
  Simple_Apu &apu = bus.apu;

  Renderer() : ui( this ) { InitEmulator(); }

  /*
  ################################
  #                              #
  #             Setup            #
  #                              #
  ################################
  */

  void InitEmulator()
  {
    auto romFile = testRoms.at( romSelected );
    bus.cartridge.LoadRom( romFile );
    cpu.Reset();
    ppu.onFrameReady = [this]( const u32 *frameBuffer ) { this->ProcessPpuFrameBuffer( frameBuffer ); };
    currentFrame = ppu.frame;

    // Set sample rate and check for out of memory error
    if ( apu.sample_rate( bus.sampleRate ) ) {
      std::cerr << "Failed to initialize APU\n";
      std::exit( EXIT_FAILURE );
    }

    // Audio
#ifdef SDL_INIT_AUDIO
    if ( SDL_Init( SDL_INIT_AUDIO ) < 0 )
      exit( EXIT_FAILURE );

    atexit( SDL_Quit );

    soundQueue = std::make_unique<Sound_Queue>();

    if ( !soundQueue )
      exit( EXIT_FAILURE );

    if ( soundQueue->init( bus.sampleRate ) )
      exit( EXIT_FAILURE );
#endif
    apu.dmc_reader( Bus::ReadDmc, &bus );

    // Directories
    recentRoms = LoadRecentROMs();
    recentRomDir = LoadRecentRomDir();
    recentStatefileDir = LoadRecentStatefileDir();

    // Bindings
    keyboardBinds = LoadKeyboardBindings();
    gamepad1Binds = LoadGamepad1Bindings();
    gamepad2Binds = LoadGamepad2Bindings();

    std::string msg = "Loaded ROM: " + std::string( romFile );
    NotifyStart( msg );
  }

  void LoadNewCartridge( const std::string &newRomFile )
  {
    if ( !bus.cartridge.IsRomValid( newRomFile ) ) {
      fmt::print( "Invalid ROM file: {}\n", newRomFile );
      return;
    }
    bus.cartridge.LoadRom( newRomFile );
    bus.DebugReset();
    currentFrame = ppu.frame;

    std::string msg = "Loaded ROM: " + std::string( newRomFile );
    NotifyStart( msg );
  }

  void OpenRomFileDialog()
  {
    const char *filters[] = { "*.nes" };
    const char *filePath = tinyfd_openFileDialog( "Choose a ROM", recentRomDir.c_str(), 1, filters, "NES ROMs", 0 );

    if ( filePath ) {
      fmt::print( "Selected ROM: {}\n", filePath );

      // Load the selected ROM
      LoadNewCartridge( filePath );

      // Remember the rom directory
      auto romDir = std::filesystem::path( filePath );
      recentRomDir = romDir.string();
      SaveRecentRomDir( romDir.string() );

      // Add to recently used roms
      AddToRecentROMs( filePath );
    }
  }

  void SaveCurrentStateFileDialog()
  {
    namespace fs = std::filesystem;

    std::string filename = "my_save" + bus.statefileExt;
    fs::path    dir = fs::path( recentStatefileDir );
    fs::path    filepath = dir / filename;

    std::string filterPattern = "*" + bus.statefileExt;
    const char *filters[] = { filterPattern.c_str() };

    // Use the string() method to get a narrow string
    const char *filePath =
        tinyfd_saveFileDialog( "Save State As", filepath.string().c_str(), 1, filters, "NES state files" );

    if ( filePath ) {
      fmt::print( "Saving state to: {}\n", filePath );
      bus.SaveState( filePath );

      // Remember the filestate directory
      recentStatefileDir = filePath;
      SaveRecentStatefileDir( recentStatefileDir );

      NotifyStart( "State save success." );
    }
  }

  bool LoadStateFileDialog()
  {
    namespace fs = std::filesystem;

    std::string filename = "my_save" + bus.statefileExt;
    fs::path    dir = fs::path( recentStatefileDir );
    fs::path    filepath = dir / filename;

    std::string filterPattern = "*" + bus.statefileExt;
    const char *filters[] = { filterPattern.c_str() };

    // Use the string() method to get a narrow string
    const char *filePath =
        tinyfd_openFileDialog( "Load State", filepath.string().c_str(), 1, filters, "NES state files", 0 );

    if ( !filePath )
      return false;

    // Verify ROM signature is from the same game
    if ( !bus.IsRomSignatureValid( filePath ) ) {
      fmt::print( "Invalid state ROM signature. Save state is likely from a different game.\n" );
      return false;
    }

    bus.LoadState( filePath );

    // Remember the filestate directory
    recentStatefileDir = filePath;
    SaveRecentStatefileDir( recentStatefileDir );

    NotifyStart( "State load success." );
    return true;
  }

  static std::deque<std::string> LoadRecentROMs()
  {
    namespace fs = std::filesystem;
    std::deque<std::string> list;
    fs::path                recentPath = fs::path( paths::user() ) / "recent";

    std::ifstream in( recentPath );
    if ( !in.is_open() )
      return list; // Nothing, return empty list

    std::string line;
    while ( std::getline( in, line ) ) {
      if ( line.empty() )
        continue;
      // Make sure ROM still exists
      if ( fs::exists( line ) ) {
        list.push_back( line );
      }
    }

    return list;
  }

  static std::string LoadRecentRomDir()
  {
    namespace fs = std::filesystem;
    fs::path      romDir = fs::path( paths::user() ) / "roms_dir";
    std::ifstream in( romDir );
    if ( !in.is_open() )
      paths::roms();

    std::string dir;
    std::getline( in, dir );
    return dir.empty() ? paths::roms() : dir;
  }

  std::string LoadRecentStatefileDir() const
  {
    namespace fs = std::filesystem;
    fs::path      statesDir = fs::path( paths::user() ) / "states_dir";
    std::ifstream in( statesDir );
    if ( in.is_open() )
      return statesDir.string();

    // default path
    fs::path path = fs::path( paths::states() ) / bus.cartridge.GetRomHash();
    if ( !fs::exists( path ) || !fs::is_directory( path ) )
      fs::create_directories( path );

    return path.string();
  }

  static void SaveRecentRomDir( const std::string &dir )
  {
    namespace fs = std::filesystem;
    fs::path      romDir = fs::path( paths::user() ) / "roms_dir";
    std::ofstream out( romDir );
    if ( !out.is_open() ) {
      std::cerr << "Failed to open recent ROMs directory file for writing.\n";
      return;
    }
    out << dir;
  }

  static void SaveRecentStatefileDir( const std::string &dir )
  {
    namespace fs = std::filesystem;
    fs::path      statesDir = fs::path( paths::user() ) / "states_dir";
    std::ofstream out( statesDir );
    if ( !out.is_open() ) {
      std::cerr << "Failed to open recent statefile directory file for writing.\n";
      return;
    }
    out << dir;
  }

  static void SaveRecentROMs( const std::deque<std::string> &list )
  {
    namespace fs = std::filesystem;
    fs::path      recentPath = fs::path( paths::user() ) / "recent";
    std::ofstream out( recentPath );
    if ( !out.is_open() ) {
      std::cerr << "Failed to open recent ROMs file for writing.\n";
      return;
    }
    for ( const auto &line : list ) {
      out << line << '\n';
    }
  }

  void AddToRecentROMs( const std::string &filePath )
  {
    if ( !bus.cartridge.IsRomValid( filePath ) ) {
      return;
    }

    auto recent = LoadRecentROMs();

    // Push the rom path to the front
    recent.erase( std::ranges::remove( recent, filePath ).begin(), recent.end() );
    recent.push_front( filePath );

    // Keep no more than 10 entries
    if ( recent.size() > 10 )
      recent.resize( 10 );

    // Save back to file
    SaveRecentROMs( recent );
    recentRoms = recent;
  }

  void ClearRecentROMs()
  {
    recentRoms.clear();
    SaveRecentROMs( recentRoms );
    NotifyStart( "Cleared recent ROMs." );
  }

  void SaveKeyboardBindings()
  {
    namespace fs = std::filesystem;
    fs::path      keyboardFile = fs::path( paths::user() ) / "keyboard";
    std::ofstream out( keyboardFile );

    if ( !out.is_open() ) {
      NotifyStart( "Failed to open keyboard settings file for writing." );
      return;
    }
    // arr to string
    std::string settings;
    for ( auto control : keyboardBinds ) {
      settings += std::to_string( control ) + " ";
    }

    out << settings;
    NotifyStart( "Keyboard settings saved." );
    std::cout << "Keyboard settings saved: " << settings << '\n';
  }

  void SaveGamepad1Bindings()
  {
    namespace fs = std::filesystem;
    fs::path      gamepad1File = fs::path( paths::user() ) / "gamepad1";
    std::ofstream out( gamepad1File );

    if ( !out.is_open() ) {
      NotifyStart( "Failed to open gamepad1 settings file for writing." );
      return;
    }
    std::string settings;
    for ( auto control : gamepad1Binds ) {
      settings += std::to_string( control ) + " ";
    }

    out << settings;
    NotifyStart( "Gamepad1 settings saved." );
  }

  void SaveGamepad2Bindings()
  {
    namespace fs = std::filesystem;
    fs::path      gamepad2File = fs::path( paths::user() ) / "gamepad2";
    std::ofstream out( gamepad2File );

    if ( !out.is_open() ) {
      NotifyStart( "Failed to open gamepad2 settings file for writing." );
      return;
    }
    std::string settings;
    for ( auto control : gamepad2Binds ) {
      settings += std::to_string( control ) + " ";
    }

    out << settings;
    NotifyStart( "Gamepad2 settings saved." );
  }

  void SaveAllBindings()
  {
    SaveKeyboardBindings();
    SaveGamepad1Bindings();
    SaveGamepad2Bindings();
  }

  void ResetDefaultBindings()
  {
    keyboardBinds = { SDL_SCANCODE_Z,  SDL_SCANCODE_X,    SDL_SCANCODE_TAB,  SDL_SCANCODE_RETURN,
                      SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT };
    gamepad1Binds = { SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
                      SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                      SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
    gamepad2Binds = { SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
                      SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
                      SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
    SaveAllBindings();
    NotifyStart( "Input settings reset to default." );
  }

  std::array<SDL_Scancode, 8> LoadKeyboardBindings()
  {
    namespace fs = std::filesystem;
    fs::path keyboardFile = fs::path( paths::user() ) / "keyboard";

    std::array<SDL_Scancode, 8> defaultBinds{ SDL_SCANCODE_X,      SDL_SCANCODE_Z,    SDL_SCANCODE_TAB,
                                              SDL_SCANCODE_RETURN, SDL_SCANCODE_UP,   SDL_SCANCODE_DOWN,
                                              SDL_SCANCODE_LEFT,   SDL_SCANCODE_RIGHT };
    std::ifstream               in( keyboardFile );
    if ( !in.is_open() ) {
      NotifyStart( "Failed to open keyboard settings file." );
      return defaultBinds;
    }

    std::string settings;
    std::getline( in, settings );

    std::array<SDL_Scancode, 8> binds{};
    std::istringstream          ss( settings );
    for ( auto &bind : binds ) {
      unsigned int tmp = 0;
      if ( !( ss >> tmp ) ) {
        NotifyStart( "Failed to load keyboard settings." );
        return defaultBinds;
      }
      bind = static_cast<SDL_Scancode>( tmp );
    }
    NotifyStart( "Keyboard settings loaded." );
    return binds;
  }

  std::array<SDL_GameControllerButton, 8> LoadGamepad1Bindings()
  {
    namespace fs = std::filesystem;
    fs::path gamepad1File = fs::path( paths::user() ) / "gamepad1";

    std::array<SDL_GameControllerButton, 8> defaultBinds{
        SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
    std::ifstream in( gamepad1File );
    if ( !in.is_open() ) {
      NotifyStart( "Failed to open gamepad1 settings file." );
      return defaultBinds;
    }

    std::string settings;
    std::getline( in, settings );

    std::array<SDL_GameControllerButton, 8> binds{};
    std::istringstream                      ss( settings );
    for ( auto &bind : binds ) {
      unsigned int tmp = 0;
      if ( !( ss >> tmp ) ) {
        NotifyStart( "Failed to load gamepad1 settings." );
        return defaultBinds;
      }
      bind = static_cast<SDL_GameControllerButton>( tmp );
    }
    NotifyStart( "Gamepad1 settings loaded." );
    return binds;
  }

  std::array<SDL_GameControllerButton, 8> LoadGamepad2Bindings()
  {
    namespace fs = std::filesystem;
    fs::path gamepad2File = fs::path( paths::user() ) / "gamepad2";

    std::array<SDL_GameControllerButton, 8> defaultBinds{
        SDL_CONTROLLER_BUTTON_A,         SDL_CONTROLLER_BUTTON_B,         SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_START,     SDL_CONTROLLER_BUTTON_DPAD_UP,   SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT };
    std::ifstream in( gamepad2File );
    if ( !in.is_open() ) {
      NotifyStart( "Failed to open gamepad2 settings file." );
      return defaultBinds;
    }

    std::string settings;
    std::getline( in, settings );

    std::array<SDL_GameControllerButton, 8> binds{};
    std::istringstream                      ss( settings );
    for ( auto &bind : binds ) {
      unsigned int tmp = 0;
      if ( !( ss >> tmp ) ) {
        NotifyStart( "Failed to load gamepad2 settings." );
        return defaultBinds;
      }
      bind = static_cast<SDL_GameControllerButton>( tmp );
    }
    NotifyStart( "Gamepad2 settings loaded." );
    return binds;
  }

  bool Setup()
  {
    if ( SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER ) != 0 ) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
      return false;
    }
    SDL_GameControllerEventState( SDL_ENABLE );

    // Decide GL+GLSL versions
#if defined( IMGUI_IMPL_OPENGL_ES2 )
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char *glslVersion = "#version 100";
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#elif defined( IMGUI_IMPL_OPENGL_ES3 )
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char *glslVersion = "#version 300 es";
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#elif defined( __APPLE__ )
    // GL 3.2 Core + GLSL 150
    const char *glslVersion = "#version 150";
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS,
                         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG ); // Always required on Mac
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#else
    // GL 3.0 + GLSL 130
    const char *glslVersion = "#version 130";
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, 0 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 0 );
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint( SDL_HINT_IME_SHOW_UI, "1" );
#endif

    /*
    ################################
    #            Window            #
    ################################
    */
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

    SDL_WindowFlags const windowFlags =
        (SDL_WindowFlags) ( SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE );
    window = SDL_CreateWindow( windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth,
                               windowHeight, windowFlags );

    if ( window == nullptr ) {
      std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
      SDL_Quit();
      return false;
    }

    glContext = SDL_GL_CreateContext( window );
    if ( glContext == nullptr ) {
      std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << '\n';
      SDL_DestroyWindow( window );
      SDL_Quit();
      return false;
    }
    SDL_GL_MakeCurrent( window, glContext );
    if ( !gladLoadGLLoader( (GLADloadproc) SDL_GL_GetProcAddress ) ) {
      std::cerr << "Failed to initialize GLAD" << '\n';
      return false;
    }
    SDL_GL_SetSwapInterval( 0 ); // disable vsync

    /*
    ################################
    ||       Game Controller      ||
    ################################
    */
    int num = SDL_NumJoysticks();
    fmt::print( "SDL reports {} joystick(s)\n", num );
    for ( int i = 0; i < num; i++ ) {
      const char *name = SDL_JoystickNameForIndex( i );
      if ( SDL_IsGameController( i ) ) {
        gamepad1 = SDL_GameControllerOpen( i );
        fmt::print( "  [{}] GameController: {}\n", i, SDL_GameControllerName( gamepad1 ) );
      } else {
        fmt::print( "  [{}] Joystick (unsupported mapping): {}\n", i, name );
      }
    }

    /*
    ################################
    #            Textures          #
    ################################
    */
    emuScreenTexture = CreateTexture( nesWidth, nesHeight );
    patternTable0Texture = CreateTexture( 128, 128 );
    patternTable1Texture = CreateTexture( 128, 128 );
    nametable0Texture = CreateTexture( 256, 240 );
    nametable1Texture = CreateTexture( 256, 240 );
    nametable2Texture = CreateTexture( 256, 240 );
    nametable3Texture = CreateTexture( 256, 240 );
    oamTexture = CreateTexture( 64, 64 );

    /*
    ################################
    #            Shader            #
    ################################
    */
    const char *vertexShaderSource = GetVertexShaderSource();
    const char *fragmentShaderSource = GetFragmentShaderSource();

    // Vertex shader
    GLuint const vertexShader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vertexShader, 1, &vertexShaderSource, nullptr );
    glCompileShader( vertexShader );
    CheckShaderCompileError( vertexShader );

    // Fragment shader
    GLuint const fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShader, 1, &fragmentShaderSource, nullptr );
    glCompileShader( fragmentShader );
    CheckShaderCompileError( fragmentShader );

    emuScreenShaderProgram = glCreateProgram();
    glAttachShader( emuScreenShaderProgram, vertexShader );
    glAttachShader( emuScreenShaderProgram, fragmentShader );
    glBindAttribLocation( emuScreenShaderProgram, 0, "a_position" );
    glBindAttribLocation( emuScreenShaderProgram, 1, "a_texcoord" );
    glLinkProgram( emuScreenShaderProgram );
    CheckShaderLinkingError( emuScreenShaderProgram );

    glDeleteShader( vertexShader );
    glDeleteShader( fragmentShader );

    // clang-format off
        // VAO and VBO
        float quadVertices[] = { // NOLINT
            // positions    // texCoords
            -1.0F,  1.0F,   0.0F, 0.0F, // top-left
            -1.0F, -1.0F,   0.0F, 1.0F, // bottom-left
             1.0F,  1.0F,   1.0F, 0.0F, // top-right
             1.0F, -1.0F,   1.0F, 1.0F  // bottom-right
        };
        glGenVertexArrays( 1, &emuScreenVAO );
        glGenBuffers( 1, &emuScreenVBO );
        glBindVertexArray( emuScreenVAO );
        glBindBuffer( GL_ARRAY_BUFFER, emuScreenVBO );
        glBufferData( GL_ARRAY_BUFFER, sizeof( quadVertices ), quadVertices, GL_STATIC_DRAW );
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), nullptr );
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof( float ), (void *) ( 2 * sizeof( float ) ) ); // NOLINT
        glBindVertexArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
    // clang-format on

    /*
    ################################
    #             ImgUI            #
    ################################
    */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    (void) io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform
                                                           // Windows

    io->ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;

    // Font setup
    ImFontConfig fontConfig;
    fontConfig.RasterizerDensity = 4.0F;
    float const fontSize = 16.0F;
    std::string fontsDir = std::string( paths::fonts() );
    fontMenu = io->Fonts->AddFontFromFileTTF( ( fontsDir + "/font-menu.otf" ).c_str(), fontSize, &fontConfig );
    fontMono = io->Fonts->AddFontFromFileTTF( ( fontsDir + "/font-mono.ttf" ).c_str(), fontSize, &fontConfig );
    fontMonoBold = io->Fonts->AddFontFromFileTTF( ( fontsDir + "/font-mono-bold.ttf" ).c_str(), fontSize, &fontConfig );

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();

    // Override specified settings, defined in theme.h
    CustomTheme::Style();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform
    // windows can look identical to regular ones. ImGuiStyle &style =
    // ImGui::GetStyle(); if ( io->ConfigFlags &
    // ImGuiConfigFlags_ViewportsEnable ) {
    //     // style.WindowRounding = 0.0F;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0F;
    // }

    // Setup Platform/Renderer backends for SDL2 + OpenGL3.
    ImGui_ImplSDL2_InitForOpenGL( window, glContext );
    ImGui_ImplOpenGL3_Init( glslVersion );

    return true;
  }

  void PlaySamples( const blip_sample_t *samples, long count ) const { soundQueue->write( samples, count ); }

  /*
  ################################
  #                              #
  #      Main Emulation Loop     #
  #                              #
  ################################
  */

  void Run()
  {
    // Compute exact NES frame interval
    constexpr double nesHz = ( 1789772.5 * 3 ) / ( 341.0 * 262.0 - 0.5 );
    auto             frameInterval = std::chrono::duration<double, std::milli>( 1000.0 / nesHz );
    auto             nextFrame = Clock::now() + frameInterval;

    while ( running ) {
      // Emulation and updates
      ExecuteFrame();
      PollEvents();
      RenderFrame();
      UpdatePatternTableTextures();
      UpdateOamTextures();
      UpdateNametableTextures();

      // Sleep until the next frame
      std::this_thread::sleep_until( nextFrame );

      // Adjust the next frame time
      nextFrame += frameInterval;

      // Catch up if behind
      auto now = Clock::now();
      if ( now > nextFrame + frameInterval ) {
        nextFrame = now + frameInterval;
      }

      // Sample FPS, cycles per second, and other metrics as needed
      SampleMetrics();

      // Keep track of frame time
      if ( now - lastFrameTime > std::chrono::seconds( 1 ) ) {
        lastFrameTime = now;
      }

      NotifyStop();
    }
  }

  void SampleMetrics()
  {
    auto now = Clock::now();
    auto delta = std::chrono::duration<double, std::milli>( now - lastSampleTime ).count();

    u64    nowCycles = cpu.GetCycles();
    double deltaCycles = (double) nowCycles - (double) lastSampleCycles;

    // sample frame times
    frameTimes.push_back( delta );
    if ( frameTimes.size() > 10 ) {
      frameTimes.erase( frameTimes.begin() );
    }
    lastSampleTime = now;

    // sample CPU cycles diffs
    cpuCycles.push_back( deltaCycles );
    if ( cpuCycles.size() > 10 ) {
      cpuCycles.erase( cpuCycles.begin() );
    }
    lastSampleCycles = nowCycles;
  }

  float GetAvgFps()
  {
    int    n = (int) frameTimes.size();
    double sum = std::accumulate( frameTimes.begin(), frameTimes.end(), 0.0 );
    double mean = sum / n;
    return static_cast<float>( 1000.0 / mean );
  }

  float GetCyclesPerSecond() const
  {
    int    n = (int) frameTimes.size();
    double sum = std::accumulate( cpuCycles.begin(), cpuCycles.end(), 0.0 );
    double mean = sum / n;
    return static_cast<float>( mean * 60.0988 );
  }

  void DebugCyclesPerSecond() const
  {
    float cyclesPerSecond = GetCyclesPerSecond();
    fmt::print( "Cycles per second: {:.2f}\n", cyclesPerSecond );
  }

  void DebugFps()
  {
    if ( frameTimes.empty() ) {
      return;
    }

    int    n = (int) frameTimes.size();
    double sum = std::accumulate( frameTimes.begin(), frameTimes.end(), 0.0 );
    double mean = sum / n;

    auto [minIt, maxIt] = std::ranges::minmax_element( frameTimes );
    double mn = *minIt;
    double mx = *maxIt;

    // population standard deviation
    double sqsum = 0.0;
    for ( double d : frameTimes ) {
      double diff = d - mean;
      sqsum += diff * diff;
    }
    double stdev = std::sqrt( sqsum / n );

    // Print to console – you can swap to fmt::print if you prefer
    std::cout << "--- FrameTiming (ms) over " << n << " samples ---\n"
              << "  mean:  " << mean << "\n"
              << "  min:   " << mn << "\n"
              << "  max:   " << mx << "\n"
              << "  stdev: " << stdev << "\n\n";
  }

  /*
  ################################
  #                              #
  #            Cleanup           #
  #                              #
  ################################
  */
  void Teardown()
  {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Delete our OpenGL texture.
    if ( emuScreenTexture ) {
      glDeleteTextures( 1, &emuScreenTexture );
      emuScreenTexture = 0;
    }
    if ( emuScreenVAO ) {
      glDeleteVertexArrays( 1, &emuScreenVAO );
      emuScreenVAO = 0;
    }
    if ( emuScreenVBO ) {
      glDeleteBuffers( 1, &emuScreenVBO );
      emuScreenVBO = 0;
    }
    if ( emuScreenShaderProgram ) {
      glDeleteProgram( emuScreenShaderProgram );
      emuScreenShaderProgram = 0;
    }

    // Destroy the OpenGL context and window.
    SDL_GL_DeleteContext( glContext );
    SDL_DestroyWindow( window );
    SDL_Quit();
  }

  /*
  ################################
  #                              #
  #         Event Polling        #
  #                              #
  ################################
  */

#if defined( __APPLE__ )
#define MOD_COMMAND KMOD_GUI
#else
#define MOD_COMMAND KMOD_CTRL
#endif

  void PollEvents()
  {
    SDL_Event event;

    while ( SDL_PollEvent( &event ) ) {
      ImGui_ImplSDL2_ProcessEvent( &event );
      if ( event.type == SDL_QUIT ) {
        running = false;
        ui.willRender = false;
      } else if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                  event.window.windowID == SDL_GetWindowID( window ) ) {
        ui.willRender = false;
        running = false;
      }
      // Gamepad1 bindings
      else if ( event.type == SDL_CONTROLLERBUTTONDOWN && isBindingGamepad1 ) {
        SDL_GameControllerButton btn = static_cast<SDL_GameControllerButton>( event.cbutton.button );
        isBindingGamepad1 = false;

        // avoid duplicates
        for ( int i = 0; i < 8; ++i )
          if ( gamepad1Binds[i] == btn && i != gamepad1Idx )
            gamepad1Binds[i] = SDL_CONTROLLER_BUTTON_INVALID;
        gamepad1Binds.at( gamepad1Idx ) = btn;
        SaveGamepad1Bindings();
        return;
      }
      // Gamepad2 bindings
      else if ( event.type == SDL_CONTROLLERBUTTONDOWN && isBindingGamepad2 ) {
        SDL_GameControllerButton btn = static_cast<SDL_GameControllerButton>( event.cbutton.button );
        isBindingGamepad2 = false;
        for ( int i = 0; i < 8; ++i )
          if ( gamepad2Binds[i] == btn && i != gamepad2Idx )
            gamepad2Binds[i] = SDL_CONTROLLER_BUTTON_INVALID;
        gamepad2Binds.at( gamepad2Idx ) = btn;
        SaveGamepad2Bindings();
        return;
      }
      // Keydown events
      else if ( event.type == SDL_KEYDOWN && event.key.repeat == 0 ) {
        const auto sc = event.key.keysym.scancode;
        const auto mods = SDL_GetModState();

        // Listen for key binding event (for keyboard input)
        if ( isBindingKeyboard ) {
          if ( sc == SDL_SCANCODE_ESCAPE ) {
            isBindingKeyboard = false;
            return;
          }

          isBindingKeyboard = false;

          // Avoid binding to keys that already do some other shortcut
          // Doing this manually won't end well, but it's fine for now.
          switch ( sc ) {
            case SDL_SCANCODE_F1  : NotifyStart( "F1 is already bound to Overlay" ); return;
            case SDL_SCANCODE_F2  : NotifyStart( "F2 is already bound to view Cartridge Info" ); return;
            case SDL_SCANCODE_KP_1: NotifyStart( "Keypad 1 is already bound to save state 1" ); return;
            case SDL_SCANCODE_KP_2: NotifyStart( "Keypad 2 is already bound to save state 2" ); return;
            case SDL_SCANCODE_KP_3: NotifyStart( "Keypad 3 is already bound to save state 3" ); return;
            default:
          }

          // Invalidate duplicates
          for ( int i = 0; i < 8; i++ ) {
            auto bind = keyboardBinds.at( i );
            if ( bind == sc && i != gamepad1Idx ) {
              keyboardBinds.at( i ) = SDL_SCANCODE_UNKNOWN;
            }
          }

          keyboardBinds.at( keyboardIdx ) = sc;
          SaveKeyboardBindings();
          return;
        }

        // cmd/ctrl + shift + key
        if ( ( mods & MOD_COMMAND ) && ( mods & KMOD_SHIFT ) ) {
          switch ( sc ) {
            case SDL_SCANCODE_S:
              fmt::print( "Save state to file\n" );
              SaveCurrentStateFileDialog();
              break;
            case SDL_SCANCODE_L:
              fmt::print( "Load state from file\n" );
              if ( LoadStateFileDialog() )
                fmt::print( "Loaded state\n" );
              else
                fmt::print( "Failed to load state\n" );
              break;
            case SDL_SCANCODE_O: {
              if ( !recentRoms.empty() && bus.cartridge.IsRomValid( recentRoms.front() ) ) {
                fmt::print( "Opening recent ROM: {}\n", recentRoms.front() );
                auto recent = recentRoms.front();
                fmt::print( "Opening recent ROM: {}\n", recent );
                LoadNewCartridge( recent );
                NotifyStart( "Opened recent ROM" );
              } else {
                NotifyStart( "No recent ROMs available." );
              }
              break;
            }
            case SDL_SCANCODE_D: ui.ToggleDebuggerWindow(); break;
            case SDL_SCANCODE_M: ui.ToggleMemory(); break;
            case SDL_SCANCODE_T: ui.ToggleLog(); break;
            case SDL_SCANCODE_C: ui.ToggleCpu(); break;
            case SDL_SCANCODE_P: ui.TogglePpu(); break;
            case SDL_SCANCODE_I: ui.ToggleInput(); break;
            default            : break;
          }
        }
        // cmd/ctrl + key
        else if ( mods & MOD_COMMAND ) {
          switch ( sc ) {
            case SDL_SCANCODE_R:
              fmt::print( "Reset\n" );
              paused = false;
              bus.DebugReset();
              NotifyStart( "Reset" );
              break;
            case SDL_SCANCODE_S:
              fmt::print( "Save state\n" );
              bus.QuickSaveState();
              NotifyStart( "State saved to slot 0." );
              break;
            case SDL_SCANCODE_L:
              fmt::print( "Load state\n" );
              bus.QuickLoadState();
              NotifyStart( "State loaded from slot 0." );
              break;
            case SDL_SCANCODE_O:
              fmt::print( "Open ROM\n" );
              OpenRomFileDialog();
              break;
            case SDL_SCANCODE_KP_1:
              bus.QuickLoadState( 1 );
              NotifyStart( "State loaded from slot 1." );
              break;
            case SDL_SCANCODE_KP_2:
              bus.QuickLoadState( 2 );
              NotifyStart( "State loaded from slot 2." );
              break;
            case SDL_SCANCODE_KP_3:
              bus.QuickLoadState( 3 );
              NotifyStart( "State loaded from slot 3." );
              break;

            case SDL_SCANCODE_1: ui.TogglePalettes(); break;
            case SDL_SCANCODE_2: ui.TogglePatternTables(); break;
            case SDL_SCANCODE_3: ui.ToggleNametables(); break;
            case SDL_SCANCODE_4: ui.ToggleSprites(); break;
            default            : break;
          }
        }
        // just key
        else {
          switch ( sc ) {
            case SDL_SCANCODE_F1: ui.ToggleOverlay(); break;
            case SDL_SCANCODE_F2: ui.ToggleCartridge(); break;

            case SDL_SCANCODE_ESCAPE:
              PauseToggle();
              NotifyStart( paused ? "Paused" : "Unpaused" );
              break;
            // num keypad 1, 2, 3 save state to slot 1, 2, 3
            case SDL_SCANCODE_KP_1:
              bus.QuickSaveState( 1 );
              NotifyStart( "State saved to slot 1." );
              break;
            case SDL_SCANCODE_KP_2:
              bus.QuickSaveState( 2 );
              NotifyStart( "State saved to slot 2." );
              break;
            case SDL_SCANCODE_KP_3:
              bus.QuickSaveState( 3 );
              NotifyStart( "State saved to slot 3." );
              break;
            default: break;
          }
        }
      } else if ( event.type == SDL_DROPFILE ) {
        char *droppedFile = event.drop.file;
        // load & track the dropped ROM:
        LoadNewCartridge( droppedFile );
        AddToRecentROMs( droppedFile );
        SDL_free( droppedFile );
      }

      // Controller device added/removed
      switch ( event.type ) { // NOLINT
        case SDL_CONTROLLERDEVICEADDED: {
          int joyIndex = event.cdevice.which;
          if ( gamepad1 == nullptr && SDL_IsGameController( joyIndex ) ) {
            gamepad1 = SDL_GameControllerOpen( joyIndex );
            fmt::print( "Controller #1 connected: {}\n", SDL_GameControllerName( gamepad1 ) );
          } else if ( gamepad2 == nullptr && SDL_IsGameController( joyIndex ) ) {
            gamepad2 = SDL_GameControllerOpen( joyIndex );
            fmt::print( "Controller #2 connected: {}\n", SDL_GameControllerName( gamepad2 ) );
          }
          break;
        }

        case SDL_CONTROLLERDEVICEREMOVED: {
          SDL_JoystickID joyId = event.cdevice.which;
          if ( gamepad1 && SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( gamepad1 ) ) == joyId ) {
            SDL_GameControllerClose( gamepad1 );
            gamepad1 = nullptr;
            fmt::print( "Controller #1 disconnected\n" );
          } else if ( gamepad2 && SDL_JoystickInstanceID( SDL_GameControllerGetJoystick( gamepad2 ) ) == joyId ) {
            SDL_GameControllerClose( gamepad2 );
            gamepad2 = nullptr;
            fmt::print( "Controller #2 disconnected\n" );
          }
          break;
        }
      }
      const Uint8 *keystate = SDL_GetKeyboardState( nullptr );

      // Map keys to controller bits
      bus.controller[0] = 0x00;
      bus.controller[1] = 0x00;

      // keyboard
      bus.controller[0] |= keystate[keyboardBinds[0]] ? 0x80 : 0x00; // A Button
      bus.controller[0] |= keystate[keyboardBinds[1]] ? 0x40 : 0x00; // B Button
      bus.controller[0] |= keystate[keyboardBinds[2]] ? 0x20 : 0x00; // Select
      bus.controller[0] |= keystate[keyboardBinds[3]] ? 0x10 : 0x00; // Start
      bus.controller[0] |= keystate[keyboardBinds[4]] ? 0x08 : 0x00; // Up
      bus.controller[0] |= keystate[keyboardBinds[5]] ? 0x04 : 0x00; // Down
      bus.controller[0] |= keystate[keyboardBinds[6]] ? 0x02 : 0x00; // Lef
      bus.controller[0] |= keystate[keyboardBinds[7]] ? 0x01 : 0x00; // Right
    }

    // gamepad 1
    if ( SDL_GameControllerGetAttached( gamepad1 ) ) {
      // clang-format off
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[0] ) ? 0x80 : 0x00; // A Button 
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[1] ) ? 0x40 : 0x00; // B Button
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[2] ) ? 0x20 : 0x00; // Select
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[3] ) ? 0x10 : 0x00; // Start
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[4] ) ? 0x08 : 0x00; // Up
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[5] ) ? 0x04 : 0x00; // Down
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[6] ) ? 0x02 : 0x00; // Left
        bus.controller[0] |= SDL_GameControllerGetButton( gamepad1, gamepad1Binds[7] ) ? 0x01 : 0x00; // Right
        // analog sticks also work
        bus.controller[0] |= SDL_GameControllerGetAxis( gamepad1, SDL_CONTROLLER_AXIS_LEFTX ) < -8000 ? 0x02 : 0x00; // Left analog
        bus.controller[0] |= SDL_GameControllerGetAxis( gamepad1, SDL_CONTROLLER_AXIS_LEFTX ) > 8000 ? 0x01 : 0x00; // Right analog
        bus.controller[0] |= SDL_GameControllerGetAxis( gamepad1, SDL_CONTROLLER_AXIS_LEFTY ) < -8000 ? 0x08 : 0x00; // Up analog
        bus.controller[0] |= SDL_GameControllerGetAxis( gamepad1, SDL_CONTROLLER_AXIS_LEFTY ) > 8000 ? 0x04 : 0x00; // Down analog
      // clang-format on
    }
  }

  /*
  ################################
  #                              #
  #            Render            #
  #                              #
  ################################
  */

  void RenderFrame()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ui.Render();

    ImGui::Render();

    int displayW = 0;
    int displayH = 0;
    int viewportX = 0;
    int viewportY = 0;
    SDL_GL_GetDrawableSize( window, &displayW, &displayH );
    ClampToAspectRatio( &viewportX, &viewportY, &displayW, &displayH );
    glViewport( viewportX, viewportY, displayW, displayH );

    glClearColor( clearColor.x, clearColor.y, clearColor.z, clearColor.w );
    glClear( GL_COLOR_BUFFER_BIT );

    // Render the 2d emulator texture
    glUseProgram( emuScreenShaderProgram );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, emuScreenTexture );
    glUniform1i( glGetUniformLocation( emuScreenShaderProgram, "u_texture" ), 0 );
    glBindVertexArray( emuScreenVAO );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    glBindVertexArray( 0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glUseProgram( 0 );
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );

    // Update and Render additional Platform Windows
    if ( io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
      SDL_Window   *backupCurrentWindow = SDL_GL_GetCurrentWindow();
      SDL_GLContext backupCurrentContext = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent( backupCurrentWindow, backupCurrentContext );
    }

    SDL_GL_SwapWindow( window );
  }

  static void ClampToAspectRatio( int *x, int *y, int *width, int *height )
  {
    float const targetAspect = 256.0F / 240.0F;
    float const windowAspect = static_cast<float>( *width ) / static_cast<float>( *height );
    int         newWidth = 0;
    int         newHeight = 0;
    int         viewportX = 0;
    int         viewportY = 0;
    if ( windowAspect > targetAspect ) {
      newHeight = *height;
      newWidth = static_cast<int>( static_cast<float>( *height ) * targetAspect );
      viewportX = ( *width - newWidth ) / 2;
    } else {
      newWidth = *width;
      newHeight = static_cast<int>( static_cast<float>( *width ) / targetAspect );
      viewportY = ( *height - newHeight ) / 2;
    }

    *x = viewportX;
    *y = viewportY;
    *width = newWidth;
    *height = newHeight;
  }

  /*
  ################################
  #                              #
  #            Helpers           #
  #                              #
  ################################
  */

  void NotifyStart( const std::string &msg, int duration = 2 )
  {
    message = msg;
    messageDuration = duration;
    messageStart = Clock::now();
    messageShow = true;
  }

  void NotifyStop()
  {
    auto now = Clock::now();
    if ( now - messageStart > std::chrono::seconds( messageDuration ) ) {
      messageShow = false;
    }
  }

  void ExecuteFrame()
  {
    while ( currentFrame == ppu.frame ) {
      if ( paused ) {
        break;
      }
      bus.Clock();
    }
    // End of frame, set the current frame to the next one.
    currentFrame = ppu.frame;

    // generate 1/60th second of sound into APU's sample buffer
    apu.end_frame();

    long count = apu.read_samples( audioBuffer, audioBufferSize );
    PlaySamples( audioBuffer, count );
  }

  void UpdateUiWindows() {}
  void UpdatePatternTableTextures()
  {
    if ( updatePatternTables ) {
      patternTable0Buffer = ppu.GetPatternTable( 0 );
      patternTable1Buffer = ppu.GetPatternTable( 1 );
    }
  }

  void UpdateNametableTextures()
  {
    if ( updateNametables ) {
      nametable0Buffer = ppu.GetNametable( 0 );
      nametable1Buffer = ppu.GetNametable( 1 );
      nametable2Buffer = ppu.GetNametable( 2 );
      nametable3Buffer = ppu.GetNametable( 3 );
    }
  }

  void UpdateOamTextures()
  {
    if ( updateOam ) {
      oamBuffer = ppu.GetOamSpriteData();
    }
  }

  GLuint GrabPatternTableTextureHandle( int tableIdx )
  {
    /*
       @brief: Updates pattern table textures, read by the cartridge from the
       PPU. Used by pattern table debug window.
    */

    GLuint const            texture = tableIdx == 0 ? patternTable0Texture : patternTable1Texture;
    std::array<u32, 16384> *frameBuffer = tableIdx == 0 ? &patternTable0Buffer : &patternTable1Buffer;

    glBindTexture( GL_TEXTURE_2D, texture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 128, 128, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer->data() );
    glBindTexture( GL_TEXTURE_2D, 0 );

    return texture;
  }

  GLuint GrabOamTextureHandle()
  {
    /*
       @brief: Updates OAM texture, read by the cartridge from the PPU. Used by
       sprite debug window.
    */
    glBindTexture( GL_TEXTURE_2D, oamTexture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 64, 64, GL_RGBA, GL_UNSIGNED_BYTE, oamBuffer.data() );
    glBindTexture( GL_TEXTURE_2D, 0 );
    return oamTexture;
  }

  GLuint GrabNametableTextureHandle( int tableIdx )
  {
    /*
       @brief: Updates nametable textures, read by the cartridge from the PPU.
       Used by nametable debug window.
    */
    GLuint const texture = tableIdx == 0   ? nametable0Texture
                           : tableIdx == 1 ? nametable1Texture
                           : tableIdx == 2 ? nametable2Texture
                                           : nametable3Texture;

    std::array<u32, 61440> *frameBuffer = tableIdx == 0   ? &nametable0Buffer
                                          : tableIdx == 1 ? &nametable1Buffer
                                          : tableIdx == 2 ? &nametable2Buffer
                                                          : &nametable3Buffer;

    glBindTexture( GL_TEXTURE_2D, texture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 256, 240, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer->data() );
    glBindTexture( GL_TEXTURE_2D, 0 );
    return texture;
  }

  void ProcessPpuFrameBuffer( const u32 *frameBuffer ) const
  {
    // Update the OpenGL texture with the new framebuffer data.
    glBindTexture( GL_TEXTURE_2D, emuScreenTexture );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, nesWidth, nesHeight, GL_RGBA, GL_UNSIGNED_BYTE, frameBuffer );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  static GLuint CreateTexture( int width, int height )
  {
    GLuint textureID = 0;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_2D, textureID );

    // Allocate the texture storage with no initial data.
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

    // Set texture filtering and wrapping options.
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    // Unbind the texture.
    glBindTexture( GL_TEXTURE_2D, 0 );

    return textureID;
  }

  static void SignalHandler( int signal )
  {
    if ( signal == SIGSEGV ) {
      std::cerr << "Segmentation Fault Detected (SIGSEGV)!\n";
      std::exit( EXIT_FAILURE );
    }
  }

  static const char *GetVertexShaderSource()
  {
#if defined( IMGUI_IMPL_OPENGL_ES2 )
    // GLSL 100 (ES2)
    return R"(
            #version 100
            attribute vec2 a_position;
            attribute vec2 a_texcoord;
            varying vec2 v_texcoord;
            void main() {
                v_texcoord = a_texcoord;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
        )";
#elif defined( IMGUI_IMPL_OPENGL_ES3 )
    // GLSL 300 es (ES3)
    return R"(
            #version 300 es
            precision mediump float;
            in vec2 a_position;
            in vec2 a_texcoord;
            out vec2 v_texcoord;
            void main() {
                v_texcoord = a_texcoord;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
        )";
#elif defined( __APPLE__ )
    // GLSL 150 (macOS)
    return R"(
            #version 150
            in vec2 a_position;
            in vec2 a_texcoord;
            out vec2 v_texcoord;
            void main() {
                v_texcoord = a_texcoord;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
        )";
#else
    // GLSL 130 (Desktop GL)
    return R"(
            #version 130
            in vec2 a_position;
            in vec2 a_texcoord;
            out vec2 v_texcoord;
            void main() {
                v_texcoord = a_texcoord;
                gl_Position = vec4(a_position, 0.0, 1.0);
            }
        )";
#endif
  }

  static const char *GetFragmentShaderSource()
  {
#if defined( IMGUI_IMPL_OPENGL_ES2 )
    // GLSL 100 (ES2)
    return R"(
            #version 100
            precision mediump float;
            varying vec2 v_texcoord;
            uniform sampler2D u_texture;
            void main() {
                gl_FragColor = texture2D(u_texture, v_texcoord);
            }
        )";
#elif defined( IMGUI_IMPL_OPENGL_ES3 )
    // GLSL 300 es (ES3)
    return R"(
            #version 300 es
            precision mediump float;
            in vec2 v_texcoord;
            uniform sampler2D u_texture;
            out vec4 outColor;
            void main() {
                outColor = texture(u_texture, v_texcoord);
            }
        )";
#elif defined( __APPLE__ )
    // GLSL 150 (macOS)
    return R"(
            #version 150
            in vec2 v_texcoord;
            uniform sampler2D u_texture;
            out vec4 outColor;
            void main() {
                outColor = texture(u_texture, v_texcoord);
            }
        )";
#else
    // GLSL 130 (Desktop GL)
    return R"(
            #version 130
            in vec2 v_texcoord;
            uniform sampler2D u_texture;
            out vec4 outColor;
            void main() {
                outColor = texture(u_texture, v_texcoord);
            }
        )";
#endif
  }

  static void CheckShaderCompileError( GLuint shader )
  {
    GLint success = 0;
    glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
    if ( !success ) {
      GLchar infoLog[512]; // NOLINT
      glGetShaderInfoLog( shader, 512, nullptr, infoLog );
      std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << '\n';
    }
  }

  static void CheckShaderLinkingError( GLuint emuScreenShaderProgram )
  {
    GLint success = 0;
    glGetProgramiv( emuScreenShaderProgram, GL_LINK_STATUS, &success );
    if ( !success ) {
      GLchar infoLog[512]; // NOLINT
      glGetProgramInfoLog( emuScreenShaderProgram, 512, nullptr, infoLog );
      std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
    }
  }
};
