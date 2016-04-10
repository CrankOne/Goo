# include <cstdlib>
# include "utest.hpp"

int
main(int argc, char * argv[]) {
    return goo::ut::UTApp::init( argc, argv, new goo::ut::UTApp( "Goo unit testing application." ) )
        ->run();
}

