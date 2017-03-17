cmake_minimum_required( VERSION 2.6 )
project(gooUtils)

option(build_unit_tests     "build testing util"        ON)
if( ENABLE_GDS )
    option(build_gdsi       "build interpreter util"    OFF)
endif( ENABLE_GDS )

if( build_unit_tests )
    add_subdirectory(UnitTests)
endif()

if( build_gdsi )
    add_subdirectory(gdsi)
endif( build_gdsi )


