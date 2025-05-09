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

namespace paths
{
inline path root()
{
  try {
    return getProjectRoot();
  } catch ( const std::filesystem::filesystem_error &e ) {
    throw std::runtime_error( "Failed to get project directory: " + std::string( e.what() ) );
  }
}

inline path assets()
{
  return root() / "assets";
}

inline std::string roms()
{
  auto path = assets() / "roms";
  return path.string();
}

inline std::string fonts()
{
  auto path = assets() / "fonts";
  return path.string();
}

inline std::string palettes()
{
  auto path = assets() / "palettes";
  return path.string();
}

inline std::string tests()
{
  auto path = root() / "tests";
  return path.string();
}

inline std::string user()
{
  auto path = root() / "user";
  return path.string();
}

inline std::string states()
{
  auto path = root() / user() / "states";
  return path.string();
}

inline std::string saves()
{
  auto path = root() / user() / "saves";
  return path.string();
}

} // namespace paths
