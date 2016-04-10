cmake_minimum_required( VERSION 2.6 )
project(gooUtils)

#\option
option(build_unit_tests     "build testing util"        ON)

if( build_unit_tests )
    add_subdirectory(UnitTests)
endif()


