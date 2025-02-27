#pragma once

#include <SDL_video.h>
#include <SDL_error.h>
#include <SDL_hints.h>
#include <SDL_events.h>
#include <fmt/base.h>
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
#include <fmt/core.h>
#include <iostream>
#include <csignal>
#include <string>
#include <memory>
#include "theme.h"

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
    GLuint        emulatorTexture = 0;
    GLuint        shaderProgram = 0;
    GLuint        vao = 0;
    GLuint        vbo = 0;
    ImGuiIO      *io{};
    ImVec4        clearColor = ImVec4( 0.00F, 0.00F, 0.00F, 1.00F );
    ImFont       *fontMenu = nullptr;
    ImFont       *fontMono = nullptr;
    ImFont       *fontMonoBold = nullptr;

    /*
    ################################
    #       global variables       #
    ################################
    */
    bool running = true;
    bool paused = false;
    u16  fps = 0;
    u64  frameCount = 0;

    /*
    ################################
    #          peripherals         #
    ################################
    */
    UIManager ui;
    Bus       bus;

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
        auto cartridge = std::make_shared<Cartridge>( "tests/roms/palette.nes" );
        bus.LoadCartridge( cartridge );
        bus.cpu.Reset();
        bus.ppu.onFrameReady = [this]( const u32 *frameBuffer ) {
            this->ProcessPpuFrameBuffer( frameBuffer );
        };
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
        window = SDL_CreateWindow( windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   windowWidth, windowHeight, windowFlags );

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
        #            Texture           #
        ################################
        */
        glGenTextures( 1, &emulatorTexture );
        glBindTexture( GL_TEXTURE_2D, emulatorTexture );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, nesWidth, nesHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glBindTexture( GL_TEXTURE_2D, 0 );

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

        shaderProgram = glCreateProgram();
        glAttachShader( shaderProgram, vertexShader );
        glAttachShader( shaderProgram, fragmentShader );
        glBindAttribLocation( shaderProgram, 0, "a_position" );
        glBindAttribLocation( shaderProgram, 1, "a_texcoord" );
        glLinkProgram( shaderProgram );
        CheckShaderLinkingError( shaderProgram );

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
        glGenVertexArrays( 1, &vao );
        glGenBuffers( 1, &vbo );
        glBindVertexArray( vao );
        glBindBuffer( GL_ARRAY_BUFFER, vbo );
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
        fontMenu = io->Fonts->AddFontFromFileTTF( "fonts/font-menu.otf", fontSize, &fontConfig );
        fontMono = io->Fonts->AddFontFromFileTTF( "fonts/font-mono.ttf", fontSize, &fontConfig );
        fontMonoBold = io->Fonts->AddFontFromFileTTF( "fonts/font-mono-bold.ttf", fontSize, &fontConfig );

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
            u64 const frameStart = SDL_GetPerformanceCounter();

            bus.cpu.ExecuteFrame( &paused );
            PollEvents();
            RenderFrame();

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
            double const secondElapsed =
                ( static_cast<double>( now - secondStart ) ) * 1000.0 / static_cast<double>( freq );
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
        if ( emulatorTexture ) {
            glDeleteTextures( 1, &emulatorTexture );
            emulatorTexture = 0;
        }
        if ( vao ) {
            glDeleteVertexArrays( 1, &vao );
            vao = 0;
        }
        if ( vbo ) {
            glDeleteBuffers( 1, &vbo );
            vbo = 0;
        }
        if ( shaderProgram ) {
            glDeleteProgram( shaderProgram );
            shaderProgram = 0;
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
            }
            if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID( window ) ) {
                ui.willRender = false;
                running = false;
            }
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
        glUseProgram( shaderProgram );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, emulatorTexture );
        glUniform1i( glGetUniformLocation( shaderProgram, "u_texture" ), 0 );
        glBindVertexArray( vao );
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
        float targetAspect = 256.0F / 240.0F;
        float windowAspect = static_cast<float>( *width ) / static_cast<float>( *height );
        int   newWidth = 0;
        int   newHeight = 0;
        int   viewportX = 0;
        int   viewportY = 0;
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
    void ProcessPpuFrameBuffer( const u32 *frameBuffer ) // NOLINT
    {
        // Update the OpenGL texture with the new framebuffer data.
        glBindTexture( GL_TEXTURE_2D, emulatorTexture );
        glPixelStorei( GL_UNPACK_ALIGNMENT, 4 );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, nesWidth, nesHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                         frameBuffer );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }

    void CalculateFps()
    {
        u64 const framesRendered = bus.ppu.GetFrame();
        u16 const framesThisSecond = framesRendered - frameCount;
        frameCount = framesRendered;
        fps = framesThisSecond;
    }

    void PrintFps() { fmt::print( "FPS: {}\n", fps ); }

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

    static void CheckShaderLinkingError( GLuint shaderProgram )
    {
        GLint success = 0;
        glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
        if ( !success ) {
            GLchar infoLog[512]; // NOLINT
            glGetProgramInfoLog( shaderProgram, 512, nullptr, infoLog );
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << '\n';
        }
    }
};
