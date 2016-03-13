cmake_minimum_required( VERSION 2.6 )
project(gooUtils)

option( build_unit_tests "unit-testing util" ON)
option( build_3dViewDemo "3D viewer application" ON)

if( build_unit_tests )
    add_subdirectory(UnitTests)
endif()

if( build_3dViewDemo )
    add_subdirectory(3dViewDemo)
endif()

