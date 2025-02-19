#include "renderer-sdl2opengl3.h"
#include <iostream>
#include <fmt/core.h>

int main()
{
    OpenGL3Renderer renderer = OpenGL3Renderer();
    try {
        renderer.Setup();
        renderer.Run();
        renderer.Teardown();
    } catch ( const std::exception &e ) {
        std::cout << e.what() << '\n';
    }
    return 0;
}
