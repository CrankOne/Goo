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

cmake_minimum_required( VERSION 2.6 )
project(GooSystemTests)

if( NOT GOO_GDB_EXEC AND NOT GOO_GCORE_EXEC )
    message( FATAL_ERROR "Couldn't find gdb nor gcore executable." )
else()
    message( STATUS "(4 systests) Found gdb ..... : ${GOO_GDB_EXEC}" )
    message( STATUS "(4 systests) Found gcore ... : ${GOO_GCORE_EXEC}" )
endif()

include_directories( "${PROJECT_SOURCE_DIR}/inc/"
                     "${PROJECT_SOURCE_DIR}/../../inc/" )
file(GLOB_RECURSE GooSysTests_SRCS src/*.c*)

set( Goo_ST_UTIL GooSTests${Goo_BUILD_POSTFIX} CACHE STRING "System tests util name" )

add_executable( ${Goo_ST_UTIL} ${GooSysTests_SRCS} )
target_link_libraries( ${Goo_ST_UTIL} ${Goo_LIBRARY} )

install( TARGETS ${Goo_ST_UTIL} RUNTIME DESTINATION bin )

