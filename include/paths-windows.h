#pragma once
#ifdef _WIN32

#include "global-types.h"
#include <windows.h>
#include <filesystem>
#include <stdexcept>

// paths-windows.h
// Used to resolve the absolute path of the executable at runtime for Windows.
// This implementation uses GetModuleFileNameA to obtain the executable's path and then returns its parent
// directory.
inline path getProjectRoot()
{
  char buffer[MAX_PATH];
  // GetModuleFileNameA returns the length of the string copied to the buffer.
  DWORD size = GetModuleFileNameA( NULL, buffer, MAX_PATH );
  if ( size == 0 ) {
    throw std::runtime_error( "Failed to retrieve executable path via GetModuleFileNameA" );
  }

  // Create a filesystem path from the buffer.
  std::filesystem::path exePath( buffer );

  // Canonicalize to resolve any symlinks and relative path elements.
  std::filesystem::path canonicalExePath = std::filesystem::canonical( exePath );

  // Return the directory containing the executable.
  return canonicalExePath.parent_path();
}
#endif
