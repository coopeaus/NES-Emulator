#pragma once

#include "bus.h"
#include "cartridge.h"
#include <SDL_stdinc.h>
#include <SDL_timer.h>
#include <cstdint>
#include <fmt/core.h>
#include <iostream>
#include <csignal>

using u32 = uint32_t;
using u64 = uint64_t;

class Renderer
{
  public:
    int         nesWidth = 256;
    int         nesHeight = 240;
    int         bufferSize = nesWidth * nesHeight;
    std::string windowTitle = "NES Emulator";
    int         windowWidth = nesWidth * 4;
    int         windowHeight = nesHeight * 4;
    bool        running = true;
    bool        renderDebugWindows = true;
    Bus         bus;
    u16         fps = 0;
    u64         frameCount = 0;

    Renderer( const Renderer & ) = delete;
    Renderer( Renderer && ) = delete;
    Renderer &operator=( const Renderer & ) = delete;
    Renderer &operator=( Renderer && ) = delete;
    virtual ~Renderer() = default;

    Renderer()
    {
        auto cartridge = std::make_shared<Cartridge>( "tests/roms/mario.nes" );
        bus.LoadCartridge( cartridge );
        bus.cpu.Reset();
        bus.ppu.onFrameReady = [this]( const u32 *frameBuffer ) {
            this->ProcessPpuFrameBuffer( frameBuffer );
        };
    }

    virtual bool Setup() = 0;
    virtual void Teardown() = 0;
    virtual void PollEvents() = 0;
    virtual void ProcessPpuFrameBuffer( const u32 *frameBuffer ) = 0;
    virtual void RenderFrame() = 0;

    void Run()
    {
        std::signal( SIGSEGV, SignalHandler );

        // Target frame time in milliseconds (~16.67ms for 60 FPS)
        const double targetFrameTimeMs = 1000.0 / 60.0;
        const u64    freq = SDL_GetPerformanceFrequency();
        u64          secondStart = SDL_GetPerformanceCounter();

        while ( running ) {
            u64 frameStart = SDL_GetPerformanceCounter();

            bus.cpu.ExecuteFrame();
            PollEvents();
            RenderFrame();

            u64    frameEnd = SDL_GetPerformanceCounter();
            double frameTimeMs =
                ( static_cast<double>( frameEnd - frameStart ) ) * 1000.0 / static_cast<double>( freq );

            // Calculate the remaining time for this frame.
            double delayTimeMs = targetFrameTimeMs - frameTimeMs;
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
            u64    now = SDL_GetPerformanceCounter();
            double secondElapsed =
                ( static_cast<double>( now - secondStart ) ) * 1000.0 / static_cast<double>( freq );
            if ( secondElapsed >= 1000.0 ) {
                CalculateFps();
                RenderFps();
                secondStart = SDL_GetPerformanceCounter();
            }
        }
    }

    void CalculateFps()
    {
        u64 framesRendered = bus.ppu.GetFrame();
        u16 framesThisSecond = framesRendered - frameCount;
        frameCount = framesRendered;
        fps = framesThisSecond;
    }

    void RenderFps() { fmt::print( "FPS: {}\n", fps ); }

    static void SignalHandler( int signal )
    {
        if ( signal == SIGSEGV ) {
            std::cerr << "Segmentation Fault Detected (SIGSEGV)!\n";
            std::exit( EXIT_FAILURE );
        }
    }
};
