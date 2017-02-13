/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# include "systest.hpp"

# include <iostream>

namespace goo {
namespace systest {

struct Config {
    int nSecsTimeout;
};

SysTestApp::SysTestApp() {}

SysTestApp::~SysTestApp() {}

Config *
SysTestApp::_V_construct_config_object( int argc, char * const argv[] ) const {
    // default config object:
    Config * cfg = new Config;

    if( 2 == argc ) {
        cfg->nSecsTimeout = atoi(argv[1]);
    } else {
        cfg->nSecsTimeout = 5;
    }

    return cfg;
}

void
SysTestApp::_V_configure_application( const Config * c ) {
    // TODO: initialize stuff here
}

std::ostream *
SysTestApp::_V_acquire_stream() {
    // TODO: return stream ptr
    return &(std::cout);
}

int
SysTestApp::_V_run() {
    printf( "Sleeping for %d seconds...\n", co().nSecsTimeout );
    sleep(co().nSecsTimeout);
    printf( "Causing a segmentation failure to receive a termination signal...\n" );
    int * aPtr = nullptr;
    int a = *aPtr;
    ((void)a);
    return EXIT_SUCCESS;
}


}  // namespace systest
}  // namespace goo

