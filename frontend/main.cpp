#include "renderer.h"
#include <exception>
#include <iostream>
#include <fmt/core.h>

int main()
{
    Renderer renderer = Renderer();
    try {
        renderer.Setup();
        renderer.Run();
        renderer.Teardown();
    } catch ( const std::exception &e ) {
        std::cout << e.what() << '\n';
    }
    return 0;
}
