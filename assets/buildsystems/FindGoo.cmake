cmake_minimum_required( VERSION 3.1 )

# - Try to find "goo" library
# Following variables should be defined once done:
#   goo_LIBRARY           ... path to the library
#   goo_INCLUDE_DIR       ... path to goo's includes dir
#   goo_C_FLAGS           ... goo's sources C-flags
#   goo_CXX_FLAGS         ... goo's sources C++-flags
# In case of using pkconfig-based configuration, there are supplementary variables:
#   pc_goo_FOUND          ... set to 1 if module(s) exist
#   pc_goo_LIBRARY        ... only the libraries (w/o the '-l')
#   pc_goo_LIBRARIES      ... only the libraries (w/o the '-l')
#   pc_goo_LIBRARY_DIRS   ... the paths of the libraries (w/o the '-L')
#   pc_goo_LDFLAGS        ... all required linker flags
#   pc_goo_LDFLAGS_OTHER  ... all other linker flags
#   pc_goo_INCLUDE_DIRS   ... the '-I' preprocessor flags (w/o the '-I')
#   pc_goo_CFLAGS         ... all required cflags
#   pc_goo_CFLAGS_OTHER   ... the other compiler flags

# Note, that if GooTests util is build some info can be retrieved by running it with -c
# flag. For example, the one-liner:
#  $ ./utils/UnitTests/GooTests -c | grep C++\ compiler | \
#                           awk -F ":" '{print $2}' | sed -e 's/^[ \t]*//;s/[ \t]*$//'
# gives the path of used C++-compiler.

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "/home/crank/Projects/goo.install/lib" isSystemDir)

# find pkg config
find_package(PkgConfig)

if( ${PKGCONFIG_FOUND} )
    if("${isSystemDir}" STREQUAL "-1")
        # doesn't wok (why? -- cmake 3.1 pkgconfig module should support it this way)
        #set( PKG_CONFIG_USE_CMAKE_PREFIX_PATH "/home/crank/Projects/goo.install/share/pkgconfig" )
        # or, for earlier CMake versions:
        set( ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:/home/crank/Projects/goo.install/share/pkgconfig" )
    endif("${isSystemDir}" STREQUAL "-1")
    pkg_check_modules(pc_goo QUIET goo)  # use prefix wc_goo...
    if("${pc_goo_FOUND}" STREQUAL "1")
        #message(STATUS "Goo includedirs:  " ${pc_goo_INCLUDE_DIRS} ", " ${pc_goo_INCLUDEDIR} )
        #message(STATUS "Goo library dirs: " ${pc_goo_LIBDIR} ", " ${pc_goo_LIBRARY_DIRS} )
        # Locate headers by goo_config.h header:
        find_path( goo_INCLUDE_DIR goo_config.h
            HINTS ${pc_goo_INCLUDE_DIRS} ${pc_goo_INCLUDEDIR}
            PATH_SUFFIXES goo )
        # locate library by libgoo:
        find_library( goo_LIBRARY NAMES goo libgoo
            HINTS ${pc_goo_LIBDIR} ${pc_goo_LIBRARY_DIRS} )
        # Now, forward execution REQUIRED/QUIET to find_package_handle_standard_args() func:
        include(FindPackageHandleStandardArgs)
        find_package_handle_standard_args(goo            DEFAULT_MSG
                                          goo_LIBRARY    goo_INCLUDE_DIR)
    else("${pc_goo_FOUND}" STREQUAL "1")
        # TODO: continue configuration without a 
        message( SEND_ERROR "pkgconfig module for goo library wasn't found." )
    endif("${pc_goo_FOUND}" STREQUAL "1")
else( ${PKGCONFIG_FOUND} )
    # TODO: configure without pkgconfig
    message( SEND_ERROR "pkgconfig isn't found. Pure CMake configuration is unimplemented yet (TODO). Sorry." )
endif( ${PKGCONFIG_FOUND} )

set( goo_C_FLAGS   "   -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99 -rdynamic"   )
set( goo_CXX_FLAGS " -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++11 -rdynamic" )

message( STATUS "Found goo library configuration:" )
message( STATUS "    built for: debug" )
message( STATUS "      library: " ${goo_LIBRARY} )
message( STATUS "  include dir: " ${goo_INCLUDE_DIR} )

#message( STATUS "    inclu#0: " ${pc_goo_DEFINITIONS} )
#message( STATUS "    inclu#1: " ${pc_goo_LIBRARIES} )
#message( STATUS "    inclu#2: " ${pc_goo_LIBRARY_DIRS} )
#message( STATUS "    inclu#3: " ${pc_goo_LDFLAGS} )
#message( STATUS "    inclu#4: " ${pc_goo_LDFLAGS_OTHER} )
#message( STATUS "    inclu#5: " ${pc_goo_INCLUDE_DIRS} )
#message( STATUS "    inclu#6: " ${pc_goo_CFLAGS} )
#message( STATUS "    inclu#7: " ${pc_goo_CFLAGS_OTHER} )
#message( STATUS "    inclu#8: " ${pc_goo_CFLAGS_OTHER} )

