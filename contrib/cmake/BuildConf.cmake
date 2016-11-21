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

# Scripts sets ${PROJECT_NAME}_BUILD_POSTFIX variable to one of the following
# variables:
#   -dbg (for `debug' dir)
#   -msr (for `minsizerel' dir)
#   -rdbg (for `relwdebinfo' dir)
#   or to empty string for euther `release' dir or custom build when build dir
# does not match to any of standard names.

#
# Attain configuration depending on current build dir.
string(REPLACE "/" ";" SPLITTED_PATH ${CMAKE_BINARY_DIR})
list(REVERSE SPLITTED_PATH)
list(GET SPLITTED_PATH 0 buildDir)
message( STATUS "Building in directory ${buildDir}" )
string( TOLOWER "${buildDir}" BUILD_DIR )
if( BUILD_DIR STREQUAL "debug" )
    message( STATUS "NOTE: debug build" )
    #\buildconf
    set( CMAKE_BUILD_TYPE "Debug" )
    set( BUILD_POSTFIX "-dbg" )
elseif( BUILD_DIR STREQUAL "release" )
    message( STATUS "NOTE: release build" )
    #\buildconf
    set( CMAKE_BUILD_TYPE "Release" )
    set( BUILD_POSTFIX "" )
elseif( BUILD_DIR STREQUAL "minsizerel" )
    message( STATUS "NOTE: minimized size build" )
    #\buildconf
    set( CMAKE_BUILD_TYPE "MinSizeRel" )
    set( BUILD_POSTFIX "-msr" )
elseif( BUILD_DIR STREQUAL "relwdebinfo" )
    message( STATUS "NOTE: release build with debugging info" )
    #\buildconf
    set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
    set( BUILD_POSTFIX "-rdbg" )
else()
    message( STATUS "NOTE: custom build" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DNDEBUG -O3" )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -DNDEBUG -O3" )
    set( BUILD_POSTFIX "" )
endif()

set( "${PROJECT_NAME}_BUILD_POSTFIX" ${BUILD_POSTFIX} )

# TODO: utilize this?
#set(VAR 
#    $<$<CONFIG:Debug>:definition_for_debug>
#    $<$<CONFIG:RelWithDebInfo>:definition_for_rel_with_debug>
#    $<$<CONFIG:Release>:definition_for_release> 
#    $<$<CONFIG:MinSizeRel>:definition_for_tight_release>
#    )

