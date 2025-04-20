#include "utils.h"
#include <cstddef>
#include <exception>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>

/*
################################
||                            ||
||      Regex log parsers     ||
||                            ||
################################
* Helper function that takes a regex match
* and returns a CPU state struct, based on the match
* Used on CPU output logs from other emulators. The logs are in different
* formats, but the general information we want from them is the same
*
*/

namespace utils
{
MatchResult parseLogLine( const std::string &line, const std::regex &pattern, size_t expectedMatches )
{
  std::smatch match;
  MatchResult fields;

  if ( regex_match( line, match, pattern ) ) {
    if ( match.size() < expectedMatches ) {
      throw std::runtime_error( "Not enough groups found in the line." );
    }
    // Skip match[0], which is the entire match.
    for ( size_t i = 1; i < match.size(); ++i ) {
      fields.push_back( match[i].str() );
    }
  } else {
    throw std::runtime_error( "Regex did not match line: " + line );
  }
  return fields;
}

MatchResults parseLog( const std::string &filename, const std::regex &pattern, size_t expectedMatches )
{
  MatchResults matches;

  std::ifstream log( filename );
  if ( !log.is_open() ) {
    throw std::runtime_error( "utils::parseLog:Error opening file: " + filename );
  }

  std::string line;
  size_t      lineNum = 0;
  while ( getline( log, line ) ) {
    try {
      matches.push_back( parseLogLine( line, pattern, expectedMatches ) );
    } catch ( const std::exception &e ) {
      throw std::runtime_error( "utils::parseLog:Error parsing line " + std::to_string( lineNum ) + ": " + e.what() );
    }
    ++lineNum;
  }

  return matches;
}
} // namespace utils
