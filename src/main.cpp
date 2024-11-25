#include "SDL2/SDL_error.h"
#include "SDL2/SDL_events.h"
#include "SDL2/SDL_render.h"
#include "SDL2/SDL_video.h"
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>

int main()
{
    if ( SDL_Init( SDL_INIT_VIDEO ) != 0 )
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow( "NES Emulator", SDL_WINDOWPOS_CENTERED,
                                           SDL_WINDOWPOS_CENTERED, 512, 480, SDL_WINDOW_SHOWN );

    if ( window == nullptr )
    {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED );
    if ( renderer == nullptr )
    {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 1;
    }
    bool      running = true;
    SDL_Event event;

    while ( running )
    {
        while ( SDL_PollEvent( &event ) != 0 )
        {
            if ( event.type == SDL_QUIT )
            {
                running = false;
            }
        }

        SDL_SetRenderDrawColor( renderer, 0, 0, 0, 255 );
        SDL_RenderClear( renderer );
        SDL_RenderPresent( renderer );
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );
    SDL_Quit();

    return 0;
}
