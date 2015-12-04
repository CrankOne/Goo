cmake_minimum_required( VERSION 2.6 )
project( Goo )

set( GOO_VERSION_MAJOR 0 )
set( GOO_VERSION_MINOR 1 )

# This script builds Goo library with all dependent
# utilities. The out-of-source build is mandatory way. Some code
# features are determined by build directory name: 'debug'
# dir enables some debugging features, while 'release'
# (default state) means that code should be optimized
# for performance.
# Right below are some build options that can be enabled or
# disabled by -D<option_name>=<ON|OFF> option given to CMake.

option( TYPES_128BIT_LENGTH     "Enable 128-bit length types."                      OFF)
option( SOURCE_POSITION_INFO    "Supplement printing lines with source file info."  OFF)
option( EM_STACK_UNWINDING      "C++ exception throw produce a stack unwinding."    ON )
option( ANSI_ESCSEQ_PRINT       "Terminal messages coloring."                       ON )
# The lines below is a development features, basically located at other
# git branches:
option( ENABLE_GDS              "Goo Declarative Semantics."                        OFF)
option( ENABLE_ALLOCATORS       "Goo's own allocators subsystem."                   OFF)
option( ENABLE_TENSORS          "Tensorial algebra support template classes."       OFF)
option( ENABLE_DATASTREAMS      "Goo's serialization template classes."             OFF)
option( ENABLE_PTREE            "Goo's getopt()-like mechanism."                    OFF)
option( HASKELL_MODULE          "Haskell extensions."                               OFF)
# Install options:
option( GOO_INSTALL_HEADERS     "Install library headers at prefix path."           ON )
option( GENERATE_PKGCFG_MODULE  "Generates pkg-config module."                      ON )
option( GENERATE_CMAKE_MODULE   "Generates library's FindGoo.cmake module."         ON )

#
# Find third-party packages
# 
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/contrib/cmake")
find_package( ZLIB  )
find_package( Lzo   )
find_package( BZip2 )
find_package( Git   )
if( EM_STACK_UNWINDING )
    find_package( BFD  REQUIRED )
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
# Add custom uninstall target (CMake by default does not provide it)
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/contrib/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake") 

#
# If installation prefix is default, set it to /usr/local
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "default install path" FORCE )
endif()

SITE_NAME( BUILDER_HOSTNAME )

include(contrib/cmake/OutOfSource.cmake)

AssureOutOfSourceBuilds()

#
# Obtain versioning parameters
    EXECUTE_PROCESS(COMMAND date +"%x %x"
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
set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -DHAVE_INLINE -std=c99" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -D_GNU_SOURCE -fexceptions -pthread -D_FILE_OFFSET_BITS=64 -std=gnu++11" )
if( CMAKE_COMPILER_IS_GNUCXX )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -rdynamic" )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic" )
endif( CMAKE_COMPILER_IS_GNUCXX )
set( CMAKE_Haskell_FLAGS "-fPIC -no-hs-main" )

set( CMAKE_C_FLAGS_DEBUG        "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer" ) # -Wfatal-errors -fprofile-use? -fprofile-correction
set( CMAKE_CXX_FLAGS_DEBUG      "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer" )
set( CMAKE_C_FLAGS_RELEASE      "${CMAKE_C_FLAGS_RELEASE}" )
set( CMAKE_CXX_FLAGS_RELEASE    "${CMAKE_CXX_FLAGS_RELEASE}" )
set( CMAKE_C_FLAGS_MINSIZEREL   "${CMAKE_C_FLAGS_MINSIZEREL}" )
set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL}" )
set( CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO}" )
set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}" )

#
# Attain configuration depending on current build dir.
string(REPLACE "/" ";" SPLITTED_PATH ${CMAKE_BINARY_DIR})
list(REVERSE SPLITTED_PATH)
list(GET SPLITTED_PATH 0 buildDir)
message( STATUS "Building in directory ${buildDir}" )
if( buildDir STREQUAL "debug" )
    message( STATUS "NOTE: debug build" )
    set( CMAKE_BUILD_TYPE "Debug" )
elseif( buildDir STREQUAL "release" )
    message( STATUS "NOTE: release build" )
    set( CMAKE_BUILD_TYPE "Release" )
elseif( buildDir STREQUAL "minsizerel" )
    message( STATUS "NOTE: minimized size build" )
    set( CMAKE_BUILD_TYPE "MinSizeRel" )
elseif( buildDir STREQUAL "relwdebinfo" )
    message( STATUS "NOTE: release build with debugging info" )
    set( CMAKE_BUILD_TYPE "RelWithDebInfo" )
else()
    message( STATUS "NOTE: custom build" )
    set( CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -DNDEBUG -O3" )
    set( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -DNDEBUG -O3" )
endif()

include_directories(
        ${GOO_LIB_INCLUDE_DIR}
    )

#
# Configure source
file(GLOB_RECURSE GOO_LIB_SOURCES src/*.c*)
configure_file (
    "${GOO_LIB_INCLUDE_DIR}/goo_config.h.in"
    "${GOO_LIB_INCLUDE_DIR}/goo_config.h"
)

if( GENERATE_PKGCFG_MODULE )
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/goo.pc.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/goo.pc"
        @ONLY
    )
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/goo.pc"
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pkgconfig )
endif( GENERATE_PKGCFG_MODULE )

if( GENERATE_CMAKE_MODULE )
    configure_file (
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/FindGoo.cmake.in"
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/FindGoo.cmake"
        @ONLY
    )
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/buildsystems/FindGoo.cmake"
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/cmake/Modules )
endif( GENERATE_CMAKE_MODULE )

if( GIT_FOUND )
    # This fake causes cmake to reconfigure itself when git versioning
    # information is changed.
    configure_file( ${CMAKE_SOURCE_DIR}/.git/COMMIT_EDITMSG
                    ${CMAKE_CURRENT_BINARY_DIR}/COMMIT_EDITMSG )
endif( GIT_FOUND )


#
# Build Goo library
add_library( goo SHARED
        ${GOO_LIB_SOURCES}
        ${BISON_GDSParser_OUTPUTS}
        ${FLEX_GDSLexer_OUTPUTS}
    )

install( TARGETS goo LIBRARY DESTINATION lib )
if( GOO_INSTALL_HEADERS )
    install( DIRECTORY inc/ DESTINATION include/goo FILES_MATCHING PATTERN
        "*.h" PATTERN "*.hpp" PATTERN "*.tcc" PATTERN "*.itcc" )
endif( GOO_INSTALL_HEADERS )

#
# Misc third-party libraries
if( ZLIB_FOUND )
    target_link_libraries( goo ${ZLIB_LIBRARIES} )
endif( ZLIB_FOUND )
if( LZO_FOUND )
    target_link_libraries( goo ${LZO_LIB} )
endif( LZO_FOUND )
if( BZIP2_FOUND )
    target_link_libraries( goo ${BZIP2_LIBRARIES} )
endif( BZIP2_FOUND )

if( EM_STACK_UNWINDING )
    if( BFD_FOUND )
        target_link_libraries( goo ${BFD_LIBRARY} )
    endif( BFD_FOUND )
    if( HASKELL_MODULE )
        target_link_libraries( goo HSrts-ghc${CMAKE_Haskell_NUMERIC_VERSION} )
    endif( HASKELL_MODULE )
    # ... other bactrace libs here
endif( EM_STACK_UNWINDING )

#
# Build enabled routines
# for build_<util> options see CMakeLists.txt in utils/
add_subdirectory(utils)

