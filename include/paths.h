// paths.h
// This module resolves absolute paths to the user's project directory at runtime.
// This assumes /assets is a subdirectory of the project root, with all the assets contained within, which
// should be copied during build time.

#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>
#include "global-types.h"

#ifdef __APPLE__
#include "paths-mac.h"
#endif

#ifdef __linux__
#include "paths-linux.h"
#endif

#ifdef _WIN32
#include "paths-windows.h"
#endif

using namespace std;

namespace paths
{
inline path root()
{
  try {
    return getProjectRoot();
  } catch ( const filesystem::filesystem_error &e ) {
    throw std::runtime_error( "Failed to get project directory: " + std::string( e.what() ) );
  }
}

inline path assets()
{
  return root() / "assets";
}

inline string roms()
{
  auto path = assets() / "roms";
  return path.string();
}

inline string fonts()
{
  auto path = assets() / "fonts";
  return path.string();
}

inline string palettes()
{
  auto path = assets() / "palettes";
  return path.string();
}

inline string tests()
{
  auto path = root() / "tests";
  return path.string();
}
} // namespace paths
