cmake_minimum_required( VERSION 2.6 )
project( gooAppExample )

# This line will find the Goo library installed in your system.
# In case you aware to install Goo in system dirs, you can specify
# a non-system prefix (e.g. ~/goo.install) and provide a path to
# GooConfig.cmake with -DGoo_DIR=~/goo.install/lib/goo/ command-line
# arguments for $ cmake command.
#
# This instruction have to introduce a few special variables into
# this CMake script:
#  Goo_INCLUDE_DIRS     - include directories for Goo library
#  Goo_LIBRARIES        - Goo libraries to link against
#  Goo_LIBRARY_DIR      - Where Goo libraries is to be placed
#  Goo_INSTALL_PREFIX   - Goo basic installation path
# and other started with Goo_ prefix.
find_package( Goo  REQUIRED )

# This line is necessary for compiler to find Goo header files:
include_directories( ${Goo_INCLUDE_DIRS} )

# This line is necessary for compiler to find Goo libraries:
link_directories( ${Goo_LIBRARY_DIR} )

# You'd probaly modify this line to add your own sources to compilation
# list (e.g. using CMake glob_recurse() command or whatever )
set( gooExampleApp_SRCS exampleApp.cpp )

# You'd probaly modify this line to add your own executable (or library) as a
# target for CMake script
add_executable( exampleApp ${gooExampleApp_SRCS} )

# This line is necessary for linker to link Goo library against your target:
target_link_libraries( exampleApp ${Goo_LIBRARIES}  )


