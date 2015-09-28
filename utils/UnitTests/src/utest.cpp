# include <iostream>
# include <iomanip>
# include <cstdlib>
# include <algorithm>
# include <sstream>
# include <cstdlib>
# include <cstdarg>

# include <getopt.h>

# include "goo_exception.hpp"
# include "goo_versioning.h"
# include "utest.hpp"

namespace goo {
namespace ut {

LabeledDAG<UTApp::TestingUnit> * UTApp::_modulesGraphPtr = nullptr;
std::unordered_map<std::string, UTApp::TestingUnit *> * UTApp::_modulesStoragePtr = nullptr;

//
// Aux
//

static void
_print_usage(const char * utilname) { fprintf(stderr, "\
GOO UNIT TESTING APPLICATION\n\r\
Syntax:\n\r\
    $ " ESC_CLRBOLD "%s" ESC_CLRCLEAR " [-h/--help] [-c/--config] [-q/--quiet] \\\n\r\
        [-l/--list] [--dot-graph] [-K/--keep-going] [-u/--selective " ESC_CLRITALIC "UNITS" ESC_CLRCLEAR "] \\\n\r\
        [-r/--report] [--ignore-deps]\n\r\
\n\r\
Provides unit testing functionality for Hephaestus library. \n\r\
\n\r\
Sequentially runs routines that exploits a partial set of library's\n\r\
functions for a bugs, side effects or other misbehaviour.\n\r\
\n\r\
Keys:\n\r\
    -h/--help ................  print this message to stdout.\n\r\
    -c/--build-config ........  print a goo's build configuration\n\r\
                                ordered in table form.\n\r\
    -q/--quiet ..............   do not print any walkthrough\n\r\
                                messages.\n\r\
    -K/--keep-going ..........  do not interrupt unit sequence\n\r\
                                walkthrough on errors.\n\r\
    -l/--list ................  print available unit names to stdout.\n\r\
    --dot-graph ..............  print to stdout a DOT graph of units.\n\r\
    -u/--selective " ESC_CLRITALIC "UNITS" ESC_CLRCLEAR " .....  run only named unit or unit in list\n\r\
                                delimeted with comma.\n\r\
    --ignore-deps ............  ignore unit dependencies (for selective runs)\n\r\
    -r/--report ..............  print development reports (unit\n\r\
                                runtime messages) to stdout.\n\r\
Report bugs to crank@qcrypt.org.\n\r\
Official repository page for this version:\n\r\
    <https://bitbucket.org/CrankOne/goo>\n\r\
Gluck auf!\n\r\
", utilname);}

static void
_print_conflicting_tasks_warning() {
    fprintf( stderr, "You're specified multiple actions. Please,\n\r\
choose exactly one of (--help/--config/--list/--dot-graph)." );
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
_set_task( Config & cfgObj, Config::Operations op ) {
    if( cfgObj.operation == Config::unassigned ) {
        cfgObj.operation = op;
    } else {
        _print_conflicting_tasks_warning();
        exit( EXIT_FAILURE );
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
    if( !_modulesGraphPtr ) {
        _modulesGraphPtr = new LabeledDAG<UTApp::TestingUnit>();
        _modulesStoragePtr = new std::unordered_map<std::string, TestingUnit *>();
    }
    _modulesGraphPtr->insert( label, unitObject );
    (*_modulesStoragePtr)[label] = unitObject;
}

Config *
UTApp::_V_construct_config_object( int argc, char * argv[] ) const {
    // default config object:
    Config * cfg = new Config {
        Config::unassigned,     // Run all modules in order.
        false,                  // Be verbose
        false,                  // Abort on unit failure.
        true,                   // Print reports about each ran unit after all.
        false,                  // Take into account unit dependencies on selective run.
    };

    {
        int c;
        while (1) {
            //int this_option_optind = optind ? optind : 1;
            int option_index = 0;
            static struct option long_options[] = {
                {"help",            no_argument,        0,  'h' },
                {"build-config",    no_argument,        0,  'c' },
                {"quiet",           no_argument,        0,  'q' },
                {"keep-going",      no_argument,        0,  'K' },
                {"report",          no_argument,        0,  'r' },
                {"list",            no_argument,        0,  'l' },
                {"selective",       required_argument,  0,  'u' },
                {"dot-graph",       no_argument,        0,   2  },
                {"ignore-deps",     no_argument,        0,   1  },
                {0,                 0,                  0,   0  }
            };

            c = getopt_long(argc, argv, "chvskrlu:",
                long_options, &option_index);
            if (c == -1)
                break;

            switch (c) {
                // OPTIONS
                case   1 : { cfg->ignoreDeps = true; } break;
                case 'K' : { cfg->keepGoing = true;  } break;
                case 'q' : { cfg->quiet = true;      } break;
                case 'r' : { cfg->printUnitsLogs = true; } break;
                // TASKS
                case 2: { _set_task( *cfg, Config::dumpDOT ); } break;
                case 'u' : {
                        _set_task( *cfg, Config::runChoosen );
                        tokenize_unit_names( optarg, cfg->names );
                    } break;
                case 'c' : { _set_task( *cfg, Config::printBuildConfig ); } break;
                case 'l' : { _set_task( *cfg, Config::listUnits ); } break;
                case 'h' :
                default : { _set_task( *cfg, Config::printHelp ); }
            }  // switch
        }  // while
    };
    if( cfg->operation == Config::unassigned ) {
        cfg->operation = Config::runAll;
    }
    return cfg;
}

void
UTApp::_V_configure_application( const Config * c ) {
    // Only continue for unit running tasks.
    if( c->operation < Config::runAll ) {
        return;
    }
    // If it is common run, just obtain the correct sequence:
    if( Config::runAll == c->operation ) {
        _modulesGraphPtr->dfs( c->units );
    }
    // If it is a selective run, determine which modules we need.
    if( Config::runChoosen == c->operation ) {
        for( auto it = c->names.begin(); c->names.end() != it; ++it ) {
            if( !c->ignoreDeps ) {
                _modulesGraphPtr->chain_for_node_by_label( *it, c->units );
            } else {
                c->units.push_front( (*_modulesGraphPtr)( *it ).data() );
            }
        }
    }
    if( !c->ignoreDeps ) {
        c->units.reverse();
    }
    c->units.unique();
}

std::ostream *
UTApp::_V_acquire_stream() {
    if( co().quiet ) {
        return (_ss = new std::stringstream());
    } else {
        return &(std::cout);
    }
}

void
UTApp::list_modules( std::ostream & os ) {
    os << "MODULES AVAILABLE:" << std::endl;
    if( _modulesGraphPtr->index().empty() ) {
        os << " <none>" << std::endl;
        return;
    }
    for( auto it = _modulesGraphPtr->index().cbegin();
              it != _modulesGraphPtr->index().cend(); ++it ) {
        os << "  - "
           << it->first
           << std::endl;
    }
}

int
UTApp::_V_run() {
    switch( UTApp::co().operation ) {
        case Config::printBuildConfig : {
            build_info();
        } break;
        case Config::listUnits : {
            list_modules( std::cout );
        } break;
        case Config::dumpDOT : {
            _modulesGraphPtr->dump_DOT_notated_graph( std::cout );
        } break;
        case Config::runAll :
        case Config::runChoosen : {
            // TODO: refine this (wrap in some kind of fancy-shmancy deco)
            if( UTApp::co().ignoreDeps ) {
                for( auto it  = UTApp::co().names.begin();
                     UTApp::co().names.end() != it; ++it ) {
                    (*_modulesStoragePtr)[(*_modulesGraphPtr)( *it ).label()]->run();
                }
            }
            // TODO
        } break;
        case Config::unassigned :
        case Config::printHelp :
        default : {
            goo::ut::_print_usage( Parent::argv[0] );
        } break;
    };
    return EXIT_SUCCESS;
}

//
// Unit
//

void
UTApp::TestingUnit::set_dependencies(
    const char [][48], uint8_t ) {
    // TODO
}

}  // namespace ut
}  // namespace goo

