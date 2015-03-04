cmake_minimum_required( VERSION 2.6 )
project(GooTests)

include_directories( "${PROJECT_SOURCE_DIR}/inc/"
                     "${PROJECT_SOURCE_DIR}/../../inc/" )
file(GLOB_RECURSE GooTests_SRCS src/*.c*)

add_executable( GooTests ${GooTests_SRCS} )
target_link_libraries( GooTests goo )

