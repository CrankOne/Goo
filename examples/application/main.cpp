# include <cstdlib>
# include "exampleApp.hpp"

// Unlike classical singletone, the goo::App<> subclasse is needed to
// be initialized. That does not much harm because of the sense of
// C main() entry point that claims a place where the main routine
// should start, so, yes, the goo::App<> needs to be constructed before
// any initialization takes place.

// The goo::App<>::init() method will perform all the initialization
// invokation in correct order (_V_configure...(), _V_acquire_stream(),
// etc.) and, finally, the run() method will invoke the _V_run().

int
main(int argc, char * argv[]) {
    return examples::ExampleApplication::init(
        argc, argv,
        new examples::ExampleApplication() )->run();
}
