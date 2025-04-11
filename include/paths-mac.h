// paths-mac.h
// Used to resolve the absolute path of the executable at runtime for macOS.
#pragma once
#ifdef __APPLE__
#include "global-types.h"
#include <mach-o/dyld.h>
#include <filesystem>
#include <stdexcept>

inline path getProjectRoot()
{
    char     buffer[PATH_MAX];
    uint32_t size = PATH_MAX;
    if ( _NSGetExecutablePath( buffer, &size ) != 0 ) {
        throw std::runtime_error( "Buffer size too small for executable path" );
    }
    path executablePath = std::filesystem::canonical( buffer );
    return executablePath.parent_path();
}
#endif
