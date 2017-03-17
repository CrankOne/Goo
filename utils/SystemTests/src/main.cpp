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

# include <cstdlib>
# include <iostream>
# include "systest.hpp"

static UByte
create_pidfile(int, siginfo_t *, void*) {
    char buf[64]; int rc;
    snprintf( buf, sizeof(buf), "touch sigint-%d.tmp", getpid() );
    rc = system(buf);
    (void) rc;
    return 0x0;
}

int
main(int argc, char * argv[]) {
    namespace ga = goo::aux;

    ga::iApp::add_handler(
            ga::iApp::_SIGINT,
            create_pidfile,
            "Testing function --- creates a file in CWD with name sigint-<pid>.tmp"
        );

    # ifdef GOO_GDB_EXEC
    ga::iApp::add_handler(
            ga::iApp::_SIGSEGV,
            ga::iApp::attach_gdb,
            "Attaches gdb to a process after SIGTERM."
        );
    # endif

    # ifdef GOO_GCORE_EXEC
    ga::iApp::add_handler(
            ga::iApp::_SIGSEGV,
            ga::iApp::dump_core,
            "Creates a core.<pid> coredump file in CWD.",
            false
        );
    # endif

    ga::iApp::dump_signal_handlers( std::cout );

    return goo::systest::SysTestApp::init(
        argc, argv, new goo::systest::SysTestApp() )->run();
}
