# include <cstdlib>
# include "utest.hpp"

int
main(int argc, char * argv[]) {
    return hphUT::Unit::run_tests( argc, argv ) ? EXIT_FAILURE : EXIT_SUCCESS;
}

