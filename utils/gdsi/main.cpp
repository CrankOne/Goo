# include <getopt.h>
# include <iostream>
# include <forward_list>

# include "goo_app.hpp"
# include "goo_versioning.h"
# include "gds/goo_interpreter.h"

static struct GDSIConf {
    uint8_t v;
    std::forward_list<std::string> files;
} _static_gdsiConf = {3};

class GDSInterpreterApplication :
    public goo::App<GDSIConf, std::ostream> {
public:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual GDSIConf * _V_construct_config_object( int argc, char * argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const GDSIConf * ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual std::ostream * _V_acquire_stream() override;
    /// Run configured application.
    virtual int _V_run() override;
};

//
// Application implementation
//

static void
_print_usage(const char * utilname) { printf("\
GOO DECLARATIVE SEMANTICS INTERPRETER\n\r\
Syntax:\n\r\
    $ %s [-h/--help] [-v/--verbosity <verbosity_level>] [file1] [file2] ...\n\r\
\n\r\
Provides standalone application implementing Goo's Declarative\n\r\
Semantics language.\n\r\
\n\r\
Simple runs scripts provided as positional arguments. If no filenames\n\r\
given, will be invoked in interactive mode.\n\r\
\n\r\
Keys:\n\r\
    [-h/--help] ..............  print this message to stdout.\n\r\
    [-v/--verbosity] .........  a verbosity level (0-3).\n\r\
Report bugs to crank@qcrypt.org.\n\r\
Official repository page for this version:\n\r\
    <https://bitbucket.org/CrankOne/goo>\n\r\
", utilname);}

GDSIConf *
GDSInterpreterApplication::_V_construct_config_object( int argc, char * argv[] ) const {
    {
        int c;
        while (1) {
            //int this_option_optind = optind ? optind : 1;
            int option_index = 0;
            static struct option long_options[] = {
                {"help",        required_argument,  0,  'h' },
                {"verbosity",   no_argument,        0,  'v' },
                {0,             0,                  0,   0  }
            };

            c = getopt_long(argc, argv, "hv:",
                long_options, &option_index);
            if (c == -1)
                break;

            switch (c) {
                case 0: {
                    printf("Uknown option %s", long_options[option_index].name);
                    if( optarg ) {
                        printf(" with arg %s", optarg);
                    }
                    printf("\n");
                } break;
                case 'v' : { _static_gdsiConf.v = atoi(optarg); } break;
                case 'h' :
                default : {
                    _print_usage(argv[0]);
                    abort();
                }
            }  // switch
        }  // while
        if( optind < argc ) {
            while (optind < argc) {
                _static_gdsiConf.files.push_front(argv[optind++]);
            }
        }
        _static_gdsiConf.files.reverse();
    };
    return &_static_gdsiConf;
}

void
GDSInterpreterApplication::_V_configure_application( const GDSIConf * c ) {
    // Do nothing here.
}

std::ostream *
GDSInterpreterApplication::_V_acquire_stream() {
    return &(std::cout);
}

int
GDSInterpreterApplication::_V_run() {
    # if 0
    struct GDS_Parser * P = gds_parser_new();
    gds_eval_string( P, "True" );
    gds_parser_destroy(P);
    # else
    if( co()->v ) {
        std::cout << "Goo Declarative Semantics interpreter application." << std::endl
                  << " Build: " << hphVersioning.buildTimestamp << ", "
                     << hphVersioning.gitCommitHash << ";" << std::endl
                  << " Features: 0x" << std::hex << hphVersioning.encodedFeatures 
                     << ";" << std::endl;
        if( _static_gdsiConf.files.empty() ) {
            std::cout << "No source files provided -- starting interactive session."
                      << std::endl;
        }
    }
    struct gds_Parser * P = gds_parser_new();
    try {
        if( _static_gdsiConf.files.empty() ) {
            // TODO: readline, eval_string(P, line)
            _TODO_
        } else {
            for( auto it = _static_gdsiConf.files.begin();
                 _static_gdsiConf.files.end() != it; ++it ) {
                gds_parser_set_filename( P, it->c_str() );
                FILE * inFile = fopen(it->c_str(), "r");
                if( !inFile ) {
                    std::cerr << "Couldn't open file \"" << *it << "\"." << std::endl;
                    continue;  // TODO: explaination.
                }
                if( co()->v ) {
                    std::cout << "Processing file \"" << *it << "\"..." << std::endl;
                }
                gds_eval_file( P, inFile );
                fclose( inFile );
            }
        }
    } catch( goo::Exception & e ) {  // generic (non-interactive) catcher.
        if( goo::Exception::gdsError == e.code() ) {
            // It's an interpreter's error: no need for full stack dump.
            std::cerr << e.what() << std::endl;
        } else {
            // Generic Goo's error:
            e.dump(std::cerr);
        }
        std::cerr << "Exit due to previous errors." << std::endl;
    }
    gds_parser_destroy(P);
    # endif

    return EXIT_SUCCESS;
}

int
main( int argc, char * argv[] ) {
    return GDSInterpreterApplication::init( argc, argv, new GDSInterpreterApplication() )
        ->run();
}

