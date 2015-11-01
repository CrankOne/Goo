cmake_minimum_required( VERSION 2.6 )
project(GooTests)

include_directories( "${PROJECT_SOURCE_DIR}/inc/"
                     "${PROJECT_SOURCE_DIR}/../../inc/" )
file(GLOB_RECURSE GooTests_SRCS src/*.c*)

add_executable( GooTests ${GooTests_SRCS} )
target_link_libraries( GooTests goo )

install( TARGETS GooTests RUNTIME DESTINATION bin )

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    message( STATUS "NOTE: custom RPATH --- " ${CMAKE_INSTALL_PREFIX} )
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    set_target_properties(GooTests PROPERTIES 
                      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
endif("${isSystemDir}" STREQUAL "-1")

