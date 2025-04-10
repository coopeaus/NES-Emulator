#pragma once
#ifdef __linux__
#include "global-types.h"
#include <filesystem>
#include <stdexcept>
#include <limits.h> //NOLINT
#include <string>
#include <unistd.h>

// Simulates your paths_linux.h
inline path getProjectRoot()
{
    char    buffer[PATH_MAX];
    ssize_t count = readlink( "/proc/self/exe", buffer, PATH_MAX );
    if ( count == -1 ) {
        throw std::runtime_error( "Failed to read /proc/self/exe" );
    }
    std::string execPathStr( buffer, count );
    path        executablePath = std::filesystem::canonical( execPathStr );
    return executablePath.parent_path();
}
#endif
