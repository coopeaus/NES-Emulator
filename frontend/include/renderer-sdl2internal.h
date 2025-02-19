#include "SDL2/SDL_error.h"
#include <SDL2/SDL_events.h>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#pragma once
#include "renderer-base.h"

class SDL2Renderer : public Renderer
{
  public:
    SDL_Window   *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture  *texture = nullptr;
    ImGuiIO       io;
    ImVec4        clearColor = ImVec4( 0.00F, 0.00F, 0.00F, 0.00F );
    bool          showDemoWindow = true;
    bool          showAnotherWindow = false;

    bool Setup() override
    {
        if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
            return true;
        }

        window = SDL_CreateWindow( windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   windowWidth, windowHeight, SDL_WINDOW_SHOWN );

        if ( window == nullptr ) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
            SDL_Quit();
            return false;
        }

        renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED );
        if ( renderer == nullptr ) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
            SDL_DestroyWindow( window );
            SDL_Quit();
            return false;
        }

        texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, nesWidth,
                                     nesHeight );

        if ( texture == nullptr ) {
            std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << '\n';
            SDL_DestroyRenderer( renderer );
            SDL_DestroyWindow( window );
            SDL_Quit();
            return true;
        }

        // Imgui Setup
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = ImGui::GetIO();
        (void) io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer( window, renderer );
        ImGui_ImplSDLRenderer2_Init( renderer );
        return true;
    }

    void Teardown() override
    {

        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyTexture( texture );
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );
        SDL_Quit();
    }

    void PollEvents() override
    {
        SDL_Event event;
        while ( SDL_PollEvent( &event ) ) {
            ImGui_ImplSDL2_ProcessEvent( &event );
            if ( event.type == SDL_QUIT ) {
                running = false;
                renderDebugWindows = false;
            }
            if ( event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                 event.window.windowID == SDL_GetWindowID( window ) ) {
                renderDebugWindows = false;
            }
        }
    }

    void ProcessPpuFrameBuffer( const u32 *frameBuffer ) override
    {
        void *pixels = nullptr;
        int   pitch = 0;

        if ( SDL_LockTexture( texture, nullptr, &pixels, &pitch ) == 0 ) {
            memcpy( pixels, frameBuffer, bufferSize * sizeof( u32 ) );
            SDL_UnlockTexture( texture );
        }
    }

    void RenderFrame() override
    {
        SDL_RenderClear( renderer );
        SDL_RenderCopy( renderer, texture, nullptr, nullptr );

        // Render ImgUI windows if active
        if ( renderDebugWindows ) {
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            if ( showDemoWindow ) {
                ImGui::ShowDemoWindow( &showDemoWindow );
            }

            ImGui::Render();
            ImGui_ImplSDLRenderer2_RenderDrawData( ImGui::GetDrawData(), renderer );
        }

        SDL_RenderPresent( renderer );
    }
};
