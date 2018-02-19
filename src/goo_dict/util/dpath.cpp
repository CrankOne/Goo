/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Created by crank on 16.01.18.
 */


# include "goo_dict/util/dpath.hpp"
# include <cstring>

namespace goo {
namespace dict {
namespace utils {

int
pull_opt_path_token( char *& path
                  , const char *& current
                  , ListIndex & idx ) {
    // if first char is digit, we'll interpret token as integer index:
    int rc = 0;
    if( '#' == *path ) {
        rc |= 0x1;
        ++path;
    }
    current = path;
    for( ; *path != '\0'; ++path ) {
        if( '.' == *path ) {
            *path =  '\0';
            ++path;
            rc |= 0x2;
            break;
        }
        if( !isalnum(*path)
            && '-' != *path
            && '_' != *path ) {
            emraise( badParameter, "Invalid path specification: character %#x "
                "is not allowed.", *path );
        }
    }
    if( 0x1 & rc ) {
        // Parse index and check its validity
        char * endPtr;
        idx = strtol( current, &endPtr, 0 );
        if( endPtr != ('\0' != *path ? path-1 : path) ) {
            emraise( badParameter, "Invalid path specification: can not"
                    " interpret \"%s\" as numerical expression.", current );
        }
        if( errno ) {
            emraise( badParameter, "Invalid path specification: strtol()"
                    " has set errno=%d: \"%s\".", errno, strerror(errno) );
        }
    }
    return rc;
}

size_t
parse_dict_path( char * pathString, DictPath * pToks, size_t nPToks ) {
    int rc;
    size_t nPTok = 0;
    do {
        rc = pull_opt_path_token( pathString, pToks->id.name, pToks->id.index );
        pToks->isIndex = 0x1 & rc;
        pToks->next = 0x2 & rc ? pToks + 1: nullptr;
        ++pToks;
    } while( 0x2 & rc && ++nPTok < nPToks );
    return nPToks;
}

std::vector<DictPath>
dpath( const std::string & path
     , std::vector<char> & namecache ) {
    // Prepare pools
    std::vector<char> & mtPath = namecache;
    mtPath.resize(path.size() + 1, '\0');
    std::vector<DictPath> toksPool;
    // Try to extract into pools
    for( size_t nToksPool = (1 << _Goo_m_PATH_TOKENS_START_2PWR)
       ; nToksPool < (1 << _Goo_m_MAX_PATH_TOKENS_2PWR)
       ; nToksPool <<= 1 ) {
        strncpy(mtPath.data(), path.c_str(), path.size() + 1);
        toksPool.clear();
        toksPool.resize( nToksPool );
        size_t nToks = parse_dict_path( mtPath.data(), toksPool.data(), toksPool.size() );
        if( nToks <= nToksPool ) {
            toksPool.resize( nToks );
            return toksPool;
        }
    }
    emraise( overflow
           , "Unable to parse path expression of length %zu since it"
             " exceeded available tokens pool (%d)."
           , path.size(), (1 << _Goo_m_MAX_PATH_TOKENS_2PWR) );
}

}  // namespace aux
}  // namespace dict
}  // namespace goo