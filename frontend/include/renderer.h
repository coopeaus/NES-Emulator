#pragma once

#include <SDL_video.h>
#include <SDL_error.h>
#include <SDL_hints.h>
#include <SDL_events.h>
#include <array>
#include <cstdlib>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "bus.h"
#include "cartridge.h"
#include "ui-component.h"
#include "ui-manager.h"
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <cstdint>
#include <fmt/base.h>
#include <fmt/core.h>
#include <iostream>
#include <csignal>
#include <string>
#include "theme.h"
#include "chrono"
#include "paths.h"

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
  #       global variables       #
  ################################
  */
  bool running = true;
  bool paused = false;
  u16  fps = 0;
  u64  frameCount = 0;

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

#define ROM( x ) ( std::string( paths::roms() ) + "/" + ( x ) )
  std::vector<std::string> testRoms = { ROM( "palette.nes" ), ROM( "color_test.nes" ),  ROM( "nestest.nes" ),
                                        ROM( "mario.nes" ),   ROM( "custom.nes" ),      ROM( "scanline.nes" ),
                                        ROM( "dk.nes" ),      ROM( "ice_climber.nes" ), ROM( "instr_test-v5.nes" ) };
  enum RomSelected : u8 { PALETTE, COLOR_TEST, NESTEST, MARIO, CUSTOM, SCANLINE, DK, ICE_CLIMBER, V5 };
  u8 romSelected = RomSelected::MARIO;

  /*
  ################################
  #          peripherals         #
  ################################
  */
  UIManager ui;
  Bus       bus;
  CPU      &cpu = bus.cpu;
  PPU      &ppu = bus.ppu;

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
  }

  void LoadNewCartridge( const std::string &newRomFile )
  {
    bus.cartridge.LoadRom( newRomFile );
    bus.DebugReset();
    currentFrame = ppu.frame;
    frameCount = 0;
  }

  bool Setup()
  {
    // Initialize SDL Video subsystem.
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER ) != 0 ) {
      std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
      return false;
    }

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
    SDL_GL_SetSwapInterval( 1 ); // Enable vsync

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
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

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

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical
    // to regular ones.
    // ImGuiStyle &style = ImGui::GetStyle();
    // if ( io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable ) {
    //     // style.WindowRounding = 0.0F;
    //     style.Colors[ImGuiCol_WindowBg].w = 1.0F;
    // }

    // Setup Platform/Renderer backends for SDL2 + OpenGL3.
    ImGui_ImplSDL2_InitForOpenGL( window, glContext );
    ImGui_ImplOpenGL3_Init( glslVersion );

    return true;
  }

  /*
  ################################
  #                              #
  #      Main Emulation Loop     #
  #                              #
  ################################
  */
  void Run()
  {
    std::signal( SIGSEGV, SignalHandler );

    // Target frame time in milliseconds (~16.67ms for 60 FPS)
    const double targetFrameTimeMs = 1000.0 / 60.0;
    const u64    freq = SDL_GetPerformanceFrequency();
    u64          secondStart = SDL_GetPerformanceCounter();

    while ( running ) {
      frameStart = Clock::now();
      u64 const frameStart = SDL_GetPerformanceCounter();

      ExecuteFrame();
      PollEvents();
      RenderFrame();
      UpdatePatternTableTextures();
      UpdateOamTextures();
      UpdateNametableTextures();

      u64 const    frameEnd = SDL_GetPerformanceCounter();
      double const frameTimeMs =
          ( static_cast<double>( frameEnd - frameStart ) ) * 1000.0 / static_cast<double>( freq );

      // Calculate the remaining time for this frame.
      double const delayTimeMs = targetFrameTimeMs - frameTimeMs;
      if ( delayTimeMs > 0 ) {
        // If there's more than ~1ms left, sleep for most of it.
        if ( delayTimeMs > 1.0 ) {
          SDL_Delay( static_cast<Uint32>( delayTimeMs - 1.0 ) );
        }
        // Busy-wait until the target frame time has elapsed.
        while ( ( ( static_cast<double>( SDL_GetPerformanceCounter() - frameStart ) ) * 1000.0 /
                  static_cast<double>( freq ) ) < targetFrameTimeMs ) {
        }
      }

      // FPS reporting every second.
      u64 const    now = SDL_GetPerformanceCounter();
      double const secondElapsed = ( static_cast<double>( now - secondStart ) ) * 1000.0 / static_cast<double>( freq );
      if ( secondElapsed >= 1000.0 ) {
        CalculateFps();
        secondStart = SDL_GetPerformanceCounter();
      }
    }
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
      } else if ( event.type == SDL_KEYDOWN ) {
        if ( event.key.repeat == 0 ) {
          // Esc to pause and unpause (for debugging)
          if ( event.key.keysym.scancode == SDL_SCANCODE_ESCAPE ) {
            paused = !paused;
            if ( paused ) {
              fmt::print( "Paused\n" );
            } else {
              fmt::print( "Unpaused\n" );
            }
          }

          // Cmd + R to reset
          if ( event.key.keysym.scancode == SDL_SCANCODE_R && ( SDL_GetModState() & KMOD_GUI ) ) {
            fmt::print( "Reset\n" );
            paused = false;
            bus.DebugReset();
          }
        }
      }
    }
    bus.controller[0] = 0x00;
    const Uint8 *keystate = SDL_GetKeyboardState( nullptr );

    // Map keys to controller bits
    bus.controller[0] |= keystate[SDL_SCANCODE_X] ? 0x80 : 0x00;     // A Button -> x
    bus.controller[0] |= keystate[SDL_SCANCODE_Z] ? 0x40 : 0x00;     // B Button -> z
    bus.controller[0] |= keystate[SDL_SCANCODE_A] ? 0x20 : 0x00;     // Select   -> a
    bus.controller[0] |= keystate[SDL_SCANCODE_S] ? 0x10 : 0x00;     // Start    -> s
    bus.controller[0] |= keystate[SDL_SCANCODE_UP] ? 0x08 : 0x00;    // Up       -> up
    bus.controller[0] |= keystate[SDL_SCANCODE_DOWN] ? 0x04 : 0x00;  // Down     -> down
    bus.controller[0] |= keystate[SDL_SCANCODE_LEFT] ? 0x02 : 0x00;  // Left     -> left
    bus.controller[0] |= keystate[SDL_SCANCODE_RIGHT] ? 0x01 : 0x00; // Right    -> right
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
  void ExecuteFrame()
  {
    while ( currentFrame == ppu.frame ) {
      if ( paused ) {
        break;
      }
      bus.Clock();
    }
    currentFrame = ppu.frame;
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
       @brief: Updates pattern table textures, read by the cartridge from the PPU. Used by pattern table
       debug window.
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
       @brief: Updates OAM texture, read by the cartridge from the PPU. Used by sprite debug window.
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
       @brief: Updates nametable textures, read by the cartridge from the PPU. Used by nametable debug
       window.
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

  void CalculateFps()
  {
    u64 const framesRendered = ppu.frame;
    u16 const framesThisSecond = framesRendered - frameCount;
    frameCount = framesRendered;
    fps = framesThisSecond;
  }

  void PrintFps() { fmt::print( "FPS: {}\n", fps ); }

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
