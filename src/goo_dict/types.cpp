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

# include "goo_dict/types.hpp"

namespace goo {
namespace dict {
namespace aux {

int
pull_opt_path_token( char *& path
                                 , char *& current
                                 , long & idx ) {
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
            fprintf( stderr, "Path specification invalid: contains "
                    "character %#x which is not allowed.\n", *path );
            abort();
        }
    }
    if( 0x1 & rc ) {
        // Parse index and check its validity
        char * endPtr;
        idx = strtol( current, &endPtr, 0 );
        if( endPtr != ('\0' != *path ? path-1 : path) ) {
            fprintf( stderr, "Path specification invalid: bad"
                            " token: \"%s\" while parsing digits.\n",
                    current );
            abort();
        }
        if( errno ) {
            fprintf( stderr, "Path specification invalid: strol()"
                    "has set errno=%d: %s\n", errno, strerror(errno) );
            abort();
        }
    }
    return rc;
}

}
}
}
