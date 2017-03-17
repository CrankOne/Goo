# include "exampleApp.hpp"

# include <iostream>

namespace examples {

struct Config {
    // Put your application-scoped configuration
    // parameters here.
};

ExampleApplication::ExampleApplication() {
    // The constructor may be used to pre-init some common stuff
    // with per-application lifetime.
    // TODO: ...
}

ExampleApplication::~ExampleApplication() {}

Config *
ExampleApplication::_V_construct_config_object( int argc, char * const argv[] ) const {
    // Create a config object:
    Config * cfg = new Config;
    // Configure cfg according to argv[]
    // TODO: ...
    return cfg;
}

void
ExampleApplication::_V_configure_application( const Config * c ) {
    // Initialize things according to finished config object here

    // TODO: ...
}

std::ostream *
ExampleApplication::_V_acquire_stream() {
    // PSets up logging streams and returns a ptr for one of them
    // (having at least one logging stream is mandatory).

    // TODO: ...

    // Here we just set the std::cout instance as our primary
    // stream.
    return &(std::cout);
}

int
ExampleApplication::_V_run() {
    // Now, application is created, application-scope configuration parameters
    // are set and real things can be evaluated.

    // TODO: ...

    // The returning result will be forwarded directly as process
    // finishing result.
    return EXIT_SUCCESS;
}


}  // namespace examples

