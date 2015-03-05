# include <cstdlib>
# include "utest.hpp"

int
main(int argc, char * argv[]) {
    goo::ut::UTApp::init( argc, argv, new goo::ut::UTApp() );
    return goo::ut::UTApp::run(); 
}

