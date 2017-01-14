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

# TODO: consider integration with native CMake functionality:
# CMAKE_DEPENDENT_OPTION
# https://cmake.org/cmake/help/v3.0/module/CMakeDependentOption.html

function( option_depend pkgName )
    if( ${pkgName} )
        foreach( dep ${ARGN} )
            if( NOT ${dep} )
                message( FATAL_ERROR "Need ${dep} to enable ${pkgName}" )
            endif()
        endforeach()
    else( ${pkgName} )
        message( STATUS "Omitting dependencies check for option ${pkgName} as it is disabled." )
    endif( ${pkgName} )
endfunction( option_depend )

