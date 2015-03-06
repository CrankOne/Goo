# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <algorithm>
# include <sstream>
# include <cstdlib>

# include <getopt.h>

# include "goo_exception.hpp"
# include "goo_versioning.h"
# include "utest.hpp"

namespace goo {
namespace ut {

LabeledDAG<UTApp::TestingUnit> UTApp::_modulesGraph;

static Config _static_Config {
    Config::runAll,     // Run all modules in order.
    false,              // Be verbose
    false,              // Abort on unit failure.
    true,               // Print reports about each ran unit after all.
    false,              // Take into account unit dependencies on selective run.
};

//
// Aux
//

static void
_print_usage(const char * utilname) { printf("\
GOO UNIT TESTING APPLICATION\n\r\
Syntax:\n\r\
    $ %s [-h/--help] [-c/--config] [-s/--silent] \\\n\r\
        [-l/--list-units] [--dot] [-k/--keep-going] [-u/--selective UNITS] \\\n\r\
        [-r/--report]\n\r\
\n\r\
Provides unit testing functionality for Hephaestus library. \n\r\
\n\r\
Sequentially runs routines that exploits a partial set of library's\n\r\
functions for a bugs, side effects or other misbehaviour.\n\r\
\n\r\
Keys:\n\r\
    [-h/--help] ..............  print this message to stdout.\n\r\
    [-c/--config] ............  print a goo's build configuration\n\r\
                                ordered in table form.\n\r\
    [-s/--silent] ............  do not print any walkthrough\n\r\
                                messages.\n\r\
    [-k/--keep-going] ........  do not interrupt unit sequence\n\r\
                                walkthrough on errors.\n\r\
    [-l/--list-units] ........  print available unit names to stdout.\n\r\
    [--dot] ..................  print to stdout a DOT graph of units.\n\r\
    [-u/--selective UNITS] ...  run only named unit or unit in list\n\r\
                                delimeted with comma.\n\r\
    [--ignore-deps] ..........  ignore unit dependencies (for selective runs)\n\r\
    [-r/--report] ............  print development reports (unit\n\r\
                                runtime messages) to stdout.\n\r\
Report bugs to crank@qcrypt.org.\n\r\
Official repository page for this version:\n\r\
    <https://bitbucket.org/CrankOne/goo>\n\r\
Gluck auf!\n\r\
", utilname);}

static void
tokenize_unit_names( const std::string & namelist,
                     std::vector<std::string> & tokens ) {
    std::stringstream ss(namelist);
    std::string item;
    while (std::getline(ss, item, ',')) {
        tokens.push_back(item);
    }
}

//
// UTApp
//

UTApp::UTApp() : _ss(nullptr) {}

UTApp::~UTApp() {
    if(_ss) {
        //delete _ss; 
    }
}

void
UTApp::register_unit( const std::string & label,
                      TestingUnit * unitObject ) {
    _modulesGraph.insert( label, unitObject );
}

Config *
UTApp::_V_construct_config_object( int argc, char * argv[] ) const {
    {
        int c;
        while (1) {
            //int this_option_optind = optind ? optind : 1;
            int option_index = 0;
            static struct option long_options[] = {
                {"help",        no_argument,        0,  'h' },
                {"config",      no_argument,        0,  'c' },
                {"silent",      no_argument,        0,  's' },
                {"keep-going",  no_argument,        0,  'k' },
                {"report",      no_argument,        0,  'r' },
                {"list-units",  no_argument,        0,  'l' },
                {"selective",   required_argument,  0,  'u' },
                {"dot",         no_argument,        0,   1  },
                {"ignore-deps", no_argument,        0,   2  },
                {0,             0,                  0,   0  }
            };

            c = getopt_long(argc, argv, "chvskrlu:",
                long_options, &option_index);
            if (c == -1)
                break;

            switch (c) {
                case 0: {
                    printf("option %s", long_options[option_index].name);
                    if( optarg ) {
                        printf(" with arg %s", optarg);
                    }
                    printf("\n");
                } break;
                case 'k' : { _static_Config.keepGoing = true; } break;
                case 's' : { _static_Config.silent = true;     } break;
                case 'r' : { _static_Config.printUnitsLogs = true; } break;
                case 'u' : {
                    _static_Config.operation = Config::runChoosen;
                    tokenize_unit_names( optarg, _static_Config.names );
                } break;
                case 'c' : { _static_Config.operation = Config::printBuildConfig; } break;
                case 'l' : { _static_Config.operation = Config::listUnits; } break;
                case 'h' :
                default : {
                    _static_Config.operation = Config::printHelp;
                }
            }  // switch
        }  // while
    };
    return &_static_Config;
}

void
UTApp::_V_configure_application( const Config * c ) {
    // Only continue for unit running tasks.
    if( c->operation < 4 ) {
        return;
    }
    // If it is common run, just obtain the correct sequence:
    if( Config::runAll == c->operation ) {
        _modulesGraph.dfs( _static_Config.units );
    }
    // If it is a selective run, determine which modules we need.
    if( Config::runChoosen == c->operation ) {
        for( auto it = c->names.begin(); c->names.end() != it; ++it ) {
            if( !c->ignoreDeps ) {
                _modulesGraph.chain_for_node_by_label( *it, _static_Config.units );
            } else {
                _static_Config.units.push_front( _modulesGraph( *it ).data() );
            }
        }
    }
    if( !c->ignoreDeps ) {
        _static_Config.units.reverse();
    }
    _static_Config.units.unique();
}

std::ostream *
UTApp::_V_acquire_stream() {
    if( co()->silent ) {
        return (_ss = new std::stringstream());
    } else {
        return &(std::cout);
    }
}

void
UTApp::list_modules( std::ostream & os ) {
    os << "MODULES AVAILABLE:" << std::endl;
    if( _modulesGraph.index().empty() ) {
        os << " <none>" << std::endl;
        return;
    }
    for( auto it = _modulesGraph.index().cbegin();
              it != _modulesGraph.index().cend(); ++it ) {
        os << "  - "
           << it->first
           << std::endl;
    }
}

int
UTApp::_V_run() {
    switch(co()->operation) {
        case Config::printBuildConfig : {
            build_info();
        } break;
        case Config::listUnits : {
            list_modules( std::cout );
        } break;
        case Config::dumpDOT : {
            _modulesGraph.dump_DOT_notated_graph( std::cout );
        } break;
        case Config::runAll :
        case Config::runChoosen : {
            // ...
        } break;
        default:
        case Config::printHelp : {
            _print_usage( Parent::argv[0] );
        } break;
    };
    return EXIT_SUCCESS;
}

}  // namespace ut
}  // namespace goo


# if 0

namespace hphUT {

std::map<std::string, Unit *> * Unit::_units = null;

void
Unit::enlist_modules( std::ostream & ) {
    std::cout << "UNIT-TESTS VAILABLE:" << std::endl;
    for( auto it = Unit::_units->cbegin();
              it != Unit::_units->cend(); ++it ) {
        std::cout << "  - "
                  << it->second->name()
                  << std::endl;
    }
}

static void
tokenize_unit_names( const std::string & namelist,
                     std::vector<std::string> & tokens ) {
    std::stringstream ss(namelist);
    std::string item;
    while (std::getline(ss, item, ',')) {
        tokens.push_back(item);
    }
}

void
Unit::run() {
    try {
        return _run(_ss);
    } catch( goo::Exception & hE ) {
        hE.dump(_ss);
    } /*catch( std::exception & sE ) {
        _ss << "caught an std::exception for unit «"
            << _name.c_str() << "»: "
            << sE.what();
    }*/
    hraise( uTestFailure, "in unit %s", _name.c_str() );
}

bool
Unit::run_tests( int argc, char * argv[] ) {
    // TODO: add support for configuration of routines
    bool keep_going = false,
         silent = false,
         devReports = false;
    std::vector<std::string> uNamesPtd;
    {
        int c;
        while (1) {
            //int this_option_optind = optind ? optind : 1;
            int option_index = 0;
            static struct option long_options[] = {
                {"help",        no_argument,        0,  'h' },
                {"config",      no_argument,        0,  'c' },
                {"silent",      no_argument,        0,  's' },
                {"keep-going",  no_argument,        0,  'k' },
                {"report",      no_argument,        0,  'r' },
                {"list-units",  no_argument,        0,  'l' },
                {"selective",   required_argument,  0,  'u' },
                {0,             0,                  0,   0  }
            };

            c = getopt_long(argc, argv, "chvskrlu:",
                long_options, &option_index);
            if (c == -1)
                break;

            switch (c) {
                case 0: {
                    printf("option %s", long_options[option_index].name);
                    if( optarg ) {
                        printf(" with arg %s", optarg);
                    }
                    printf("\n");
                } break;
                case 'k' : { keep_going = true; } break;
                case 's' : { silent = true;     } break;
                case 'c' : { build_info();      } break;
                case 'r' : { devReports = true; } break;
                case 'u' : {
                    tokenize_unit_names( optarg, uNamesPtd );
                } break;
                case 'l' : {
                    Unit::enlist_modules(std::cout);
                    return EXIT_FAILURE;
                } break;
                case 'h' :
                default : {
                    _print_usage( argv[0] );
                    return EXIT_FAILURE;
                }
            }  // switch
        }  // while
    };

    # ifndef NO_STACKTRACE
    if( !silent ){
        printf( ESC_BLDWHITE "  NOTE (stack unwinding):" ESC_CLRCLEAR "\n" );
        printf( "    Application may use switching stacks when obtaining backtrace info.\n");
        printf( "    If you're using valgrind, please, append \"--max-stackframe=2098736\"\n");
        printf( "    or greater in order to avoid alarms about switched stacks.\n" );
        # ifndef NO_BFD_LIB
        printf( ESC_BLDWHITE "  NOTE (using BFD):" ESC_CLRCLEAR "\n" );
        printf( "    An exception's stacktrace info obtained by Hephaestus's routines can be\n");
        printf( "    wrong for current executable when running under debuggers.\n");
        #endif
    }
    # endif

    // run
    Size nRuns = 0,
              nFails = 0;
    if( !silent ) {
        std::cout << ESC_BLDWHITE " == Haephestus unit testing application start" ESC_CLRCLEAR << std::endl;
        std::cout << std::setw(25) << std::right << "units known : " ESC_BLDWHITE ;
        std::cout << _units->size() << ESC_CLRCLEAR << std::endl;
    }


    std::map<std::string, std::string> failureReports;
    for( auto it = _units->begin(); it != _units->end(); ++it, ++nRuns ) {
        if( !uNamesPtd.empty() ) {
            if( std::find( uNamesPtd.begin(),
                           uNamesPtd.end(), it->first ) == uNamesPtd.end() ) {
                continue;
            }
        }
        try {
            if( !silent ) {
                std::cout << ESC_BLDGREEN << std::setw(45) << std::right;
                std::cout << it->first << ESC_CLRCLEAR" ... ";
            }

            it->second->run();  // < engage!

            if( !silent ) {
                std::cout << ESC_BLDBLUE "SUCCESS" ESC_CLRCLEAR << std::endl;
            }
            if( devReports ) {
                std::cout << it->second->get_report();
            }
        } catch( goo::Exception & e ) {
            if( !silent ) {
                std::cout <<  ESC_BLDRED "FAILURE" ESC_CLRCLEAR << std::endl;
            }
            ++nFails;
            if( !keep_going ) {
                return true;
            } else {
                failureReports[it->second->name()] = it->second->get_report();
            }
        }
    }

    if( !silent ) {
        std::cout << ESC_BLDWHITE " == Haephestus unit testing done" ESC_CLRCLEAR << std::endl;
        std::cout << std::setw(25) << std::right << "units ran : " ESC_BLDWHITE ;
        std::cout << nRuns << ESC_CLRCLEAR << std::endl;
        std::cout << std::setw(25) << std::right << "failures : " ESC_BLDWHITE ;
        std::cout << nFails << ESC_CLRCLEAR << std::endl;
        std::cout << ESC_BLDWHITE " == Haephestus unit testing application done" ESC_CLRCLEAR << std::endl;
        if( nFails ) {  // print reports
            std::cout << ESC_BLDRED "== FAILURE REPORTS BGN" ESC_CLRCLEAR << std::endl;
            for( auto it = failureReports.begin();
                    it != failureReports.end(); ++it) {
                std::cout << ESC_BLDYELLOW "* Unit report <<" << it->first << ">> BGN" ESC_CLRCLEAR << std::endl;
                std::cout << it->second << std::endl;
                std::cout << ESC_BLDYELLOW "* Unit report <<" << it->first << ">> END" ESC_CLRCLEAR << std::endl;
            }
            std::cout << ESC_BLDRED "== FAILURE REPORTS END" ESC_CLRCLEAR << std::endl;
        }
    }

    return nFails; // false -- ok
}

}  // namespace HphST

# endif

