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
MatchResult parseLogLine( const string &line, const regex &pattern, size_t expectedMatches )
{
    smatch      match;
    MatchResult fields;

    if ( regex_match( line, match, pattern ) ) {
        if ( match.size() < expectedMatches ) {
            throw runtime_error( "Not enough groups found in the line." );
        }
        // Skip match[0], which is the entire match.
        for ( size_t i = 1; i < match.size(); ++i ) {
            fields.push_back( match[i].str() );
        }
    } else {
        throw runtime_error( "Regex did not match line: " + line );
    }
    return fields;
}

MatchResults parseLog( const string &filename, const regex &pattern, size_t expectedMatches )
{
    MatchResults matches;

    ifstream log( filename );
    if ( !log.is_open() ) {
        throw runtime_error( "utils::parseLog:Error opening file: " + filename );
    }

    string line;
    size_t lineNum = 0;
    while ( getline( log, line ) ) {
        try {
            matches.push_back( parseLogLine( line, pattern, expectedMatches ) );
        } catch ( const exception &e ) {
            throw runtime_error( "utils::parseLog:Error parsing line " + to_string( lineNum ) + ": " +
                                 e.what() );
        }
        ++lineNum;
    }

    return matches;
}
} // namespace utils
