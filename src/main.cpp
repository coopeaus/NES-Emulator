#include "SDL2/SDL_error.h"
#include "SDL2/SDL_pixels.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_video.h"
#include "bus.h"
#include "cartridge.h"
#include <cstdint>
#include <cstring>
#include <memory>
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>
#include <csignal>

using u32 = uint32_t;
using namespace std;

constexpr int SCREEN_WIDTH = 512;  // NOLINT
constexpr int SCREEN_HEIGHT = 480; // NOLINT
constexpr int NES_WIDTH = 256;     // NOLINT
constexpr int NES_HEIGHT = 240;    // NOLINT
constexpr int BUFFER_SIZE = 61440; // NOLINT

SDL_Texture *texture = nullptr; // NOLINT

void emulation();
void signalHandler( int signal );
void renderFrame( const u32 *frameBufferData );

int main()
{
    /*
    ################################
    ||                            ||
    ||       Initialize SDL       ||
    ||                            ||
    ################################
    */
    std::signal( SIGSEGV, signalHandler );

    if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow( "NES Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                           SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );

    if ( window == nullptr ) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if ( renderer == nullptr ) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }

    texture = SDL_CreateTexture( renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, NES_WIDTH,
                                 NES_HEIGHT );

    if ( texture == nullptr ) {
        std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer( renderer );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }

    SDL_Event event;

    /*
    ################################
    ||                            ||
    ||     Initialize Emulator    ||
    ||                            ||
    ################################
    */
    // Initiate the emulator
    Bus bus;

    /*
    ################################
    ||          Load ROM          ||
    ################################
    */
    shared_ptr<Cartridge> const cartridge = make_shared<Cartridge>( "tests/roms/mario.nes" );
    bus.LoadCartridge( cartridge );
    bus.cpu.Reset();

    /*
    ################################
    ||     PPU Render Callback    ||
    ################################
    */
    bus.ppu.onFrameReady = renderFrame;

    /*
    ################################
    ||            Time            ||
    ################################
    */
    using Clock = chrono::high_resolution_clock;
    auto start = Clock::now();
    auto lastFpsTime = Clock::now();
    auto lastPollTime = Clock::now();
    auto lastRenderTime = Clock::now();
    auto now = Clock::now();
    auto fpsElapsed = chrono::duration_cast<chrono::milliseconds>( now - start ).count();
    auto pollElapsed = chrono::duration_cast<chrono::milliseconds>( now - start ).count();
    auto renderElapsed = chrono::duration_cast<chrono::milliseconds>( now - start ).count();
    u16  lastFrame = 0; // Stores the last known frame count

    /*
    ################################
    ||                            ||
    ||       Emulation Loop       ||
    ||                            ||
    ################################
    */
    bool running = true;
    while ( running ) {
        bus.cpu.DecodeExecute();
        now = Clock::now();

        /*
        ################################
        ||             FPS            ||
        ################################
        */
        fpsElapsed = chrono::duration_cast<chrono::milliseconds>( now - lastFpsTime ).count();
        if ( fpsElapsed >= 1000 ) {
            u16 currentFrame = bus.ppu.GetFrame();
            u16 framesRendered = currentFrame - lastFrame; // Calculate FPS
            cout << "FPS: " << framesRendered << '\n';

            lastFrame = currentFrame; // Store current frame count
            lastFpsTime = now;        // Reset FPS timer
        }

        /*
        ################################
        ||         SDL Polling        ||
        ################################
        */
        pollElapsed = chrono::duration_cast<chrono::milliseconds>( now - lastPollTime ).count();
        if ( pollElapsed >= 16 ) {

            while ( SDL_PollEvent( &event ) ) {
                switch ( event.type ) {
                    case SDL_QUIT:
                        running = false;
                    default:
                        break;
                }
            }
            lastPollTime = now;
        }

        /*
        ################################
        ||          Rendering         ||
        ################################
        */
        renderElapsed = chrono::duration_cast<chrono::milliseconds>( now - lastRenderTime ).count();
        if ( renderElapsed >= 16 ) {
            SDL_RenderClear( renderer );
            SDL_RenderCopy( renderer, texture, nullptr, nullptr );
            SDL_RenderPresent( renderer );
            lastRenderTime = now;
        }
    }

    // Cleanup
    SDL_DestroyTexture( texture );
    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}

/*
################################
||                            ||
||           Helpers          ||
||                            ||
################################
*/

void renderFrame( const u32 *frameBufferData ) // NOLINT
{
    void *pixels = nullptr;
    int   pitch = 0;

    if ( SDL_LockTexture( texture, nullptr, &pixels, &pitch ) == 0 ) {
        memcpy( pixels, frameBufferData, BUFFER_SIZE * sizeof( u32 ) );
        SDL_UnlockTexture( texture );
    }
}

void signalHandler( int signal )
{
    if ( signal == SIGSEGV ) {
        std::cerr << "Segmentation Fault Detected (SIGSEGV)!\n";
        std::exit( EXIT_FAILURE );
    }
}
