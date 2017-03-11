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

# - Find LZO (lzo.h, liblzo2.a)
# This module defines
#  LZO_INCLUDE_DIR, directory containing headers
#  LZO_STATIC_LIB, path to libslzo2.a
#  LZO_FOUND, whether lzo has been found

find_path(LZO_INCLUDE_DIR NAMES lzo/lzoconf.h)

find_library(LZO_LIB  NAMES liblzo2.so)

if (LZO_LIB)
  if (LZO_INCLUDE_DIR)
    set(LZO_FOUND TRUE)
  else ()
    set(LZO_FOUND FALSE)
  endif()
else ()
  set(LZO_FOUND FALSE)
endif ()

if (LZO_FOUND)
  if (NOT LZO_FIND_QUIETLY)
    message(STATUS "Lzo Library ${LZO_LIB}")
    message(STATUS "Lzo Include Found in ${LZO_INCLUDE_DIR}")
  endif ()
else ()
  message(STATUS "Lzo includes and libraries NOT found. ")
endif ()

mark_as_advanced(
  LZO_INCLUDE_DIR
  LZO_LIBS
)
