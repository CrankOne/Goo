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
project( Goo )

# TODO: read this:
# https://cmake.org/cmake/help/v3.1/manual/cmake-packages.7.html#manual:cmake-packages(7)
# to know about how to correctly make the redistributable package and handle
# external dependencies.

# Goo versioning:
#   - The major (digit) indexes the Goo's revision number and may imply
#   a massive changes of API.
#   - The minor (digit) indexes the releases and pre-releasing versions.
#   Setting it to odd numbers (1, 3, 5, etc.) implies that current version
#   is a development one and different patches may not follow a fixed API.
#   In contrast, setting minor number to even numbers (0, 2, 4, etc.)
#   implies that this is a release version where API have not be changed
#   without crusial need.
#   - The patch postfix : alphanumerical identifier used to indicate latest
#   patch applied for release version. For development minor numbers its
#   form is arbitrary.
set( Goo_VERSION_MAJOR 1  )
set( Goo_VERSION_MINOR 0  )
set( Goo_VERSION_PATCH pure )
set( Goo_VERSION ${Goo_VERSION_MAJOR}.${Goo_VERSION_MINOR}.${Goo_VERSION_PATCH} )

include(contrib/cmake/OptionList.cmake)

# This script builds Goo library with all dependent
# utilities. The out-of-source build is mandatory way. Some code
# features are determined by build directory name: 'debug'
# dir enables some debugging features, while 'release'
# (default state) means that code should be optimized
# for performance.
# Right below are some build options that can be enabled or
# disabled by -D<option_name>=<ON|OFF> option given to CMake.

#\option
push_option( TYPES_128BIT_LENGTH
        "Enable 128-bit length types."
        OFF)
#\option
push_option( SOURCE_POSITION_INFO
        "Supplement printing lines with source file info."
        OFF)
#\option
push_option( EM_STACK_UNWINDING
        "C++ exception throw produce a stack unwinding."
        ON )
#\option
push_option( ANSI_ESCSEQ_PRINT
        "Terminal messages coloring."
        ON )
# The lines below is a development features, basically located at other
# git branches:
#\option
push_option( ENABLE_GDS
        "Goo Declarative Semantics."
        OFF )
#\option
push_option( ENABLE_ALLOCATORS
        "Goo's own allocators subsystem."
        OFF )
#\option
push_option( ENABLE_TENSORS
        "Tensorial algebra support template classes."
        OFF )
#\option
push_option( ENABLE_DATASTREAMS
        "Goo's serialization template classes."
        OFF )
#\option
push_option( ENABLE_PTREE
        "Goo's getopt()-like mechanism."
        OFF )
#\option
push_option( HASKELL_MODULE
        "Haskell extensions."
        OFF )
#\option
push_option( BUILD_STATIC_LIBRARIES
        "Enables build of static versions of libraries also"
        OFF )

#
# Find third-party packages
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/contrib/cmake)
find_package( ZLIB  )
find_package( Lzo   )
find_package( BZip2 )
find_package( Git   )
find_program( GOO_GDB_EXEC      gdb     )
find_program( GOO_GCORE_EXEC    gcore   )
if( EM_STACK_UNWINDING )
    find_package( BFD REQUIRED )
    find_package( LibIberty REQUIRED )
endif( EM_STACK_UNWINDING )
if( GIT_FOUND )
    # obtain current commit hash
    EXECUTE_PROCESS(COMMAND git rev-parse HEAD OUTPUT_VARIABLE COMMIT_HASH
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_STRIP_TRAILING_WHITESPACE )
    EXECUTE_PROCESS(COMMAND git rev-parse --abbrev-ref HEAD
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                    OUTPUT_VARIABLE GIT_BRANCH_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
    EXECUTE_PROCESS(COMMAND git show -s
        --format="gitInfo:{\\%nCommit:{\\%nhash:%h,\\%ndate:'%ci',\\%ncomment:'%s',\\%n},\\%nbranch:'${GIT_BRANCH_NAME}'}" ${COMMIT_HASH}
                 WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                 OUTPUT_VARIABLE GIT_STRING OUTPUT_STRIP_TRAILING_WHITESPACE)
else( GIT_FOUND )
    SET( COMMIT_HASH "<unknown>" )
    SET( GIT_STRING "gitInfo:'unavailable';" )
endif( GIT_FOUND )

#
# If installation prefix is default, set it to /usr/local
if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE )
    message( STATUS "NOTE: Install prefix is set to \""
        ${CMAKE_INSTALL_PREFIX} "\" as it was set to default. "
        "The -rpath won't be specified as it appears to be a "
        "system path."
        "To override this specification, use -DCMAKE_INSTALL_PREFIX=<path> "
        "option." )
else( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
    message( STATUS "NOTE: Install prefix is manually set to \""
        ${CMAKE_INSTALL_PREFIX} "\"." )
    ## ...
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/goo")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    # the RPATH to be used when installing, but only if it's not a system directory
    #list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
    #message( STATUS "XXX - ${isSystemDir}" )  # -1 when it is not a system dir
endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )

#
# Add custom uninstall target (CMake by default does not provide it)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/CMakeUninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake"
    IMMEDIATE @ONLY)
add_custom_target(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake") 

site_name( BUILDER_HOSTNAME )

include(contrib/cmake/OutOfSource.cmake)

AssureOutOfSourceBuilds()

#
# Obtain versioning parameters
execute_process(COMMAND date +"%x %x"
                OUTPUT_VARIABLE BUILD_TIMESTAMP
                OUTPUT_STRIP_TRAILING_WHITESPACE )

#
# Configure grammar generator (for HDS, if not disabled)
if( ENABLE_GDS )
    find_package(BISON)
    find_package(FLEX)
    # Some distros have no this modules
    if( NOT BISON_FOUND )
        message( WARNING "BISON cmake-module not found; trying to use custom module" )
        include(contrib/cmake/FindBISON.custom)
    endif( NOT BISON_FOUND )
    if( NOT FLEX_FOUND )
        message( WARNING "FLEX cmake-module not found; trying to use custom module" )
        include(contrib/cmake/FindFLEX.custom)
    endif( NOT FLEX_FOUND )
    BISON_TARGET(GDSParser contrib/grammar/gds.y
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gds_parser.generated.c
        VERBOSE ${CMAKE_CURRENT_BINARY_DIR}/gds.dbg
        COMPILE_FLAGS "--graph=${CMAKE_CURRENT_BINARY_DIR}/gds.dot" )
    FLEX_TARGET(GDSLexer   contrib/grammar/gds.l ${CMAKE_CURRENT_SOURCE_DIR}/src/gds_lexer.generated.c)
    ADD_FLEX_BISON_DEPENDENCY(GDSLexer GDSParser)
endif( ENABLE_GDS )

#
# Workaround some harmless compiler warnings about generated code.
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/gds_lexer.generated.c  PROPERTIES COMPILE_FLAGS -Wno-sign-compare)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/src/gds_parser.generated.c PROPERTIES COMPILE_FLAGS -Wno-write-strings)

set( GOO_LIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/inc/" )

#
# Configure compiler -- base flags
# NOTE: for ASan, prepend with -fsanitize=address -fno-omit-frame-pointer
#set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99" )
#set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++1y " )
if( CMAKE_COMPILER_IS_GNUCXX )
        set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -rdynamic -I/usr/include/libiberty/" )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -I/usr/include/libiberty/" )
endif( CMAKE_COMPILER_IS_GNUCXX )
set( CMAKE_Haskell_FLAGS "-fPIC -no-hs-main" )

set( CMAKE_C_FLAGS_DEBUG            "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer" )
set( CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer" )
set( CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE}" )
set( CMAKE_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

#
# Attain configuration depending on current build dir.
include(contrib/cmake/BuildConf.cmake)
set( Goo_LIBRARY goo${Goo_BUILD_POSTFIX} )
if( BUILD_STATIC_LIBRARIES )
    set( Goo_StaticLIBRARY goo${Goo_BUILD_POSTFIX}-static )
endif( BUILD_STATIC_LIBRARIES )

#
# Configure source
file(GLOB_RECURSE GOO_LIB_SOURCES src/*.c*)
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/goo_config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/goo_config.h
)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/goo.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/goo.pc"
    @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/goo.pc"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig )

# This path refers to CMake modules installation
set( GOO_CMAKE_MODULES_DESTINATION lib/cmake/goo )

#
# Build Goo library
add_library( ${Goo_LIBRARY} SHARED
        ${GOO_LIB_SOURCES}
        ${BISON_GDSParser_OUTPUTS}
        ${FLEX_GDSLexer_OUTPUTS}
    )
set_target_properties(${Goo_LIBRARY} PROPERTIES
    VERSION ${Goo_VERSION}
    SOVERSION ${Goo_VERSION_MAJOR}
    INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib/goo )

target_include_directories( ${Goo_LIBRARY} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include/goo> )
target_compile_features( ${Goo_LIBRARY} PUBLIC
            c_variadic_macros
            cxx_constexpr
            cxx_final
            cxx_delegating_constructors
            cxx_variadic_macros
            cxx_variadic_templates
            cxx_template_template_parameters )

target_compile_definitions( ${Goo_LIBRARY} PUBLIC _GNU_SOURCE _FILE_OFFSET_BITS=64 )
target_compile_options( ${Goo_LIBRARY} PRIVATE -Wall -fexceptions -pthread )
# Note this useful snippet to list compiler features:
#get_property(cxx_features GLOBAL PROPERTY CMAKE_CXX_KNOWN_FEATURES)
#foreach(feature ${cxx_features})
#    message(" * ${feature}")
#endforeach()

install( TARGETS ${Goo_LIBRARY} EXPORT goo LIBRARY DESTINATION lib/goo )

if( BUILD_STATIC_LIBRARIES )
    add_library( ${Goo_StaticLIBRARY} STATIC
        ${GOO_LIB_SOURCES}
        ${BISON_GDSParser_OUTPUTS}
        ${FLEX_GDSLexer_OUTPUTS}
    )
    install( TARGETS ${Goo_StaticLIBRARY} EXPORT goo ARCHIVE DESTINATION lib/goo )
endif( BUILD_STATIC_LIBRARIES )

install( DIRECTORY inc/ DESTINATION include/goo FILES_MATCHING PATTERN
    "*.h" PATTERN "*.hpp" PATTERN "*.tcc" PATTERN "*.itcc" )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/goo_config.h"
    DESTINATION include/goo )

#
# Misc third-party libraries
if( ZLIB_FOUND )
    target_link_libraries( ${Goo_LIBRARY} PUBLIC ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )
if( LZO_FOUND )
    target_link_libraries( ${Goo_LIBRARY} PUBLIC ${LZO_LIB} )
endif( LZO_FOUND )
if( BZIP2_FOUND )
    target_link_libraries( ${Goo_LIBRARY} PUBLIC ${BZIP2_LIBRARIES} )
endif( BZIP2_FOUND )

if( EM_STACK_UNWINDING )
    if( BFD_FOUND )
        target_link_libraries( ${Goo_LIBRARY} PRIVATE ${BFD_LIBRARY} )
    endif( BFD_FOUND )
    if( HASKELL_MODULE )
        target_link_libraries( ${Goo_LIBRARY} PUBLIC HSrts-ghc${CMAKE_Haskell_NUMERIC_VERSION} )
    endif( HASKELL_MODULE )
    # ... other backtrace libs here
endif( EM_STACK_UNWINDING )

#
# Build enabled routines
# for build_<util> options see CMakeLists.txt in utils/
#\project-subdir
add_subdirectory(utils)

#
# CMake module/config outputs:
if( build_unit_tests )
    set( Goo_UT_EXECUTABLE  ${CMAKE_INSTALL_PREFIX}/bin/${Goo_UT_UTIL} )
else( build_unit_tests )
    set( Goo_UT_EXECUTABLE  Goo_UT_EXECUTABLE-NOTFOUND )
endif( build_unit_tests )

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/FindGoo.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FindGoo.cmake"
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/FindGoo.cmake
        DESTINATION lib/goo )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GooConfigVersion.cmake
    VERSION ${Goo_VERSION}
    COMPATIBILITY AnyNewerVersion )
options_list()  #< collects the ${PKG_NAME}_OPTIONS_STR variable
export( EXPORT goo
  FILE "${CMAKE_CURRENT_BINARY_DIR}/GooTargets.cmake"
)
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/GooConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/GooConfig.cmake"
    @ONLY
)

#
# Copy some useful contributed/vendor modules:
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/GooConfigVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/GooConfig.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/OutOfSource.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/CMakeUninstall.cmake.in
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/OptionDepend.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/BuildConf.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/OptionList.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/PyExtensions.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/PyExtensions_init-template.in
    DESTINATION ${GOO_CMAKE_MODULES_DESTINATION} )

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/dev/testbuild.pl"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin
)

install( EXPORT goo DESTINATION lib/cmake/goo
         FILE GooTargets.cmake )

