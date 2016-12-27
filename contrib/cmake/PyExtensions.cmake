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

# TODO: doc following the
# https://cmake.org/cmake/help/v3.4/manual/cmake-developer.7.html#module-documentation

# First, before function declaration make CMake memorize the location of
# current module to further access to PyExtensions_init-template.in. See:
# http://stackoverflow.com/questions/12802377/in-cmake-how-can-i-find-the-directory-of-an-included-file
# for explaination why do we need that:
set( THIS_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR} )

# Function signature:
# goo_py_extensions( INTERFACES <iface1> <iface2> ...
#                    [PKG_NAME <package name>]
#                    [LINK_LIBS <lib1> <lib2> ...]
#                    [NO_INIT_FILE] )
# Please, note that one can specify Python_ADDITIONAL_VERSIONS to point out
# desired versions of python to be used. Like that:
#   set( Python_ADDITIONAL_VERSIONS 2.7 2.6 2.5 2.4 )
# or from cmd-line with -D... syntax like:
# $ cmake ... -DPython_ADDITIONAL_VERSIONS="2.7;2.6;2.5;2.4"
function( goo_py_extensions )
    # Function signature:
    set( options NO_INIT_FILE )
    set( oneValueArgs PKG_NAME )
    set( multiValueArgs INTERFACES LINK_LIBS )
    cmake_parse_arguments( py_extensions
            "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    if( py_extensions_PKG_NAME )
        set( PYPKGNM ${py_extensions_PKG_NAME} )
    else( py_extensions_PKG_NAME )
        set( PYPKGNM ${PROJECT_NAME} )
    endif( py_extensions_PKG_NAME )

    find_package( SWIG REQUIRED )
    find_package( PythonInterp REQUIRED )
    find_package( PythonLibs REQUIRED )

    include( ${SWIG_USE_FILE} )
    set( CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -Wall )
    include_directories( ${PYTHON_INCLUDE_DIRS} )

    #
    # Choose the install destination.
    # - if default installation path is specified or PYTHON_SITE_PACKAGES var
    #   is provided:
    if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR PYTHON_SITE_PACKAGES )
        if( NOT PYTHON_SITE_PACKAGES )
            # Get system site packages dir for current interpreter if no
            # PYTHON_SITE_PACKAGES var was provided:
            execute_process(
                COMMAND ${PYTHON_EXECUTABLE} -c "from __future__ import print_function; from distutils.sysconfig import get_python_lib; print get_python_lib()"
                OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE )
        endif( NOT PYTHON_SITE_PACKAGES )
        set( PY_MODULES_INSTALL_DIR ${PYTHON_SITE_PACKAGES} )
    else( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
        # Dunno whether it is legal:
        string( REPLACE "." ";" VERSION_LIST ${PYTHONLIBS_VERSION_STRING} )
        list( GET VERSION_LIST 0 PYTHON_VERSION_MAJOR)
        list( GET VERSION_LIST 1 PYTHON_VERSION_MINOR)
        set( PY_VER "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}" )
        set( PY_MODULES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/python${PY_VER}/site-packages )
    endif( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT OR PYTHON_SITE_PACKAGES )
    message( STATUS "Python site-packages dir is set to "
                        "\"${PY_MODULES_INSTALL_DIR}\"." )

    if( NOT py_extensions_INTERFACES )
        message( FATAL_ERROR
                 "No SWIG-interface modules were provided for goo_py_extensions().")
    endif( NOT py_extensions_INTERFACES )

    # Now generate a list of modules
    foreach( IFACE_FILE IN LISTS py_extensions_INTERFACES )
        get_filename_component( IFACE ${IFACE_FILE} NAME_WE )
        message( STATUS "Located extension module \"${IFACE}\" at ${IFACE_FILE}..." )
        ## ${SWIG_MODULE_${name}_REAL_NAME}.
        set_source_files_properties( ${IFACE_FILE} PROPERTIES CPLUSPLUS ON )
        swig_add_module( ${IFACE} python ${IFACE_FILE} )
        swig_link_libraries( ${IFACE} ${PYTHON_LIBRARIES} ${py_extensions_LINK_LIBS})
        install(TARGETS ${SWIG_MODULE_${IFACE}_REAL_NAME}
                DESTINATION ${PY_MODULES_INSTALL_DIR}/${PYPKGNM} )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${IFACE}.py
                DESTINATION ${PY_MODULES_INSTALL_DIR}/${PYPKGNM} )
        set( ${PYPKGNM}_PY_MODULES "${${PYPKGNM}_PY_MODULES}${IFACE} " )
    endforeach()

    # Form __all__ variable in __init__.py file according to
    # enabled targets
    if( NOT py_extensions_NO_INIT_FILE )
        message( STATUS "__init__.py will be generated for ${PYPKGNM}" )
        configure_file( ${THIS_MODULE_DIR}/PyExtensions_init-template.in
                        ${CMAKE_CURRENT_BINARY_DIR}/__init__.py )
        install( FILES ${CMAKE_CURRENT_BINARY_DIR}/__init__.py
                 DESTINATION ${PY_MODULES_INSTALL_DIR}/${PYPKGNM} )
    endif( NOT py_extensions_NO_INIT_FILE )

    set( PY_MODULES_INSTALL_DIR ${PY_MODULES_INSTALL_DIR} PARENT_SCOPE )
    set( PYPKGNM ${PYPKGNM} PARENT_SCOPE )
endfunction( goo_py_extensions )


