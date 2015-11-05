# include <cstdlib>
# include "utest.hpp"

int
main(int argc, char * const argv[]) {
    goo::ut::UTApp::init( argc, argv, new goo::ut::UTApp( argv[0] ) );
    return goo::ut::UTApp::run(); 
}

