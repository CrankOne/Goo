# Goo application example

The motivation to object-oriented representation for application
mostly comes from paradigm of object oriented programming itself.

Most of the parts of the Goo library does not rely on `goo::iApp`
singleton instance according to the way this library is composed.
Nevertheless the goo::App class provides some non-obtrusive
facility for signal handling environmental variables processing,
so few classes in Goo can require the Goo app to be instantiated.

# Example

This example demonstrates basic usage of `Goo::App<>` template class
defining few necessary methods in most basic way. One can use this
as a raw text template for defining a custom application
implementtion.

It contains five files:

    ./examples/application/     # this directory
    ├── CMakeLists.txt          # CMake script for building the app
    ├── exampleApp.cpp          # the implementation file
    ├── exampleApp.hpp          # the declaration file
    ├── main.cpp                # entry point implementation file
    └── README.md               # this readme file

which are complete example of minimal application. See comments in
sources for details.

