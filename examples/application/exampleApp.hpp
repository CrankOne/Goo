
# ifndef H_GOO_SYSTEST_H
# define H_GOO_SYSTEST_H

# include "goo_app.hpp"

namespace examples {

// Configuration structure. Can be an object of arbitrary type;
// will be returned by .co() method of SysTestApp instance.
struct Config;

// The goo::App<> template class takes two template arguments:
//  - the config object type (whatever you need);
//  - an output stream object (usually std::ostream or
//    its descendant);
class ExampleApplication : public goo::App<Config, std::ostream> {
protected:

    // Mandatory interface method --- creates instance of type config
    // object according to command-line arguments.
    virtual Config * _V_construct_config_object( int argc, char * const argv[] ) const override;

    // Mandatory interface method --- 
    // Configures application according to constructed config object
    // like opening files, connections, preparing workspaces, etc.
    virtual void _V_configure_application( const Config * ) override;

    // Mandatory interface method --- creates common logging stream.
    // For the purpose of simplicity, basically
    // one stream is envisaged by default. One can, however, easily
    // extend this behaviour with other streams initializations
    // here (i.e. in similar way it is organized in standard lib ---
    // std::cout/std::cerr).
    virtual std::ostream * _V_acquire_stream() override;

    // Mandatory interface method --- an actual entry point that will be
    // invoked after configuration stage is passed.
    virtual int _V_run() override;
public:
    // Constructor.
    ExampleApplication();
    
    // Destructor that will be invoked if everything finished ok or
    // all the exceptions were handled.
    ~ExampleApplication();
};  // class ExampleApplication

}  // namespace examples

# endif  // H_GOO_SYSTEST_H

