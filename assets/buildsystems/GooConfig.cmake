# Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
# Author: Renat R. Dusaev <crank@qcrypt.org>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# CMake config file for Goo library package
# - It defines the following variables
#  Goo_INCLUDE_DIRS     - include directories for Goo library
#  Goo_LIBRARIES        - Goo libraries to link against
#  Goo_LIBRARY_DIR      - Where Goo libraries is to be placed
#  Goo_CC               - Goo C compiler
#  Goo_C_CFLAGS         - Goo C compiler flags
#  Goo_CXX              - Goo C++ compiler
#  Goo_CXX_CFLAGS       - Goo C++ compiler flags
#  Goo_INSTALL_PREFIX   - Goo basic installation path
# - If the unit test utility is enabled to build the following variables are
# to be defined:
#  Goo_UT_EXECUTABLE    - the Goo unit tests executable

set( Goo_INCLUDE_DIRS   "/home/crank/Projects/goo.install/include/goo" )
set( Goo_LIBRARIES      goo-dbg )
set( Goo_LIBRARY_DIR    /home/crank/Projects/goo.install/lib/goo )
set( Goo_INSTALL_PREFIX /home/crank/Projects/goo.install )

set( Goo_CC             "/usr/bin/cc" )
set( Goo_C_CFLAGS       "   -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99 -rdynamic"   )

set( Goo_CXX            "/usr/bin/c++" )
set( Goo_CXX_CFLAGS     " -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++11 -rdynamic" )

set( Goo_UT_EXECUTABLE  /home/crank/Projects/goo.install/bin/GooTests-dbg )

