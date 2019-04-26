/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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

/// Unit test application config object.
struct Config {
    /// Available functions of application. For descriptions, see _V_construct_config_object().
    enum Operations {
        unassigned          = 0,
        printHelp           = 1,
        printBuildConfig    = 2,
        listUnits           = 3,
        dumpDOT             = 4,
        runAll              = 5,
        runChoosen          = 6,
    } operation;

    /// Supplementary options.
    bool quiet,
         keepGoing,
         printUnitsLogs,
         ignoreDeps,
         noCatch;

    /// Vector of unit names desired to run.
    std::vector<std::string> namesToEvaluate,
                             namesToAvoid;

    /// Prepared unit sequence to run.
    mutable LabeledDAG<UTApp::TestingUnit>::Order units;
};

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
Provides unit testing functionality for Goo library. \n\r\
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
    -s/--skip " ESC_CLRITALIC "UNITS" ESC_CLRCLEAR " ..........  skip selected units.\n\r\
    --ignore-deps ............  ignore unit dependencies (for selective runs)\n\r\
    -r/--report ..............  print development reports (unit\n\r\
                                runtime messages) to stdout.\n\r\
    -E/--no-catch ............  do not catch any unexpected exceptions during\n\r\
                                unit(s) run\n\r\
Report bugs to crank@qcrypt.org.\n\r\
Official repository page for this version:\n\r\
    <https://github.com/CrankOne/Goo>\n\r\
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

UTApp::UTApp( const std::string & appName ) : _ss(nullptr), _appName(appName) {}

UTApp::~UTApp() {
    // free units:
    for( auto it = _modulesStoragePtr->begin();
         _modulesStoragePtr && _modulesStoragePtr->end() != it;
         ++it ) {
        delete it->second;
    }
    // free containers:
    if( _modulesGraphPtr ) {
        delete _modulesGraphPtr;
    }
    if( _modulesStoragePtr ) {
        delete _modulesStoragePtr;
    }
    // free stringstream, if it wasn't set to standard objects:
    if(_ss && _ss != &(std::cout) && _ss != &(std::cerr)) {
        delete _ss;
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
UTApp::_V_construct_config_object( int argc, char * const argv[] ) const {
    // default config object:
    Config * cfg = new Config {
        Config::unassigned,     // Run all modules in order.
        false,                  // Be verbose
        false,                  // Abort on unit failure.
        false,                  // Print reports about each ran unit after all.
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
                {"skip",            required_argument,  0,  's' },
                {"no-catch",        no_argument,        0,  'E' },
                {"dot-graph",       no_argument,        0,   2  },
                {"ignore-deps",     no_argument,        0,   1  },
                {0,                 0,                  0,   0  }
            };
            c = getopt_long(argc, argv, "hcqKrlsEu:s:",
                long_options, &option_index);
            if (c == -1)
                break;

            switch (c) {
                // OPTIONS
                case   1 : { cfg->ignoreDeps = true; } break;
                case 'K' : { cfg->keepGoing = true;  } break;
                case 'q' : { cfg->quiet = true;      } break;
                case 'r' : { cfg->printUnitsLogs = true; } break;
                case 'E' : { cfg->noCatch = true; } break;
                // TASKS
                case 2: { _set_task( *cfg, Config::dumpDOT ); } break;
                case 'u' : {
                        _set_task( *cfg, Config::runChoosen );
                        tokenize_unit_names( optarg, cfg->namesToEvaluate );
                    } break;
                case 's' : {
                        tokenize_unit_names( optarg, cfg->namesToAvoid );
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
    // Implying that all units now ready, set up dependency relations now.
    _incorporate_dependencies();
    // Only continue for unit running tasks.
    if( c->operation < Config::runAll ) {
        return;
    }
    if( !_modulesGraphPtr ) {
        emraise( badState, "No modules graph structured yet. Couldn't continue." );
    }
    // If it is common run, just obtain the correct sequence:
    if( Config::runAll == c->operation ) {
        _modulesGraphPtr->dfs( c->units );
    }
    // If it is a selective run, determine which modules we need.
    if( Config::runChoosen == c->operation ) {
        // For all units pointed out:
        for( auto it = c->namesToEvaluate.begin(); c->namesToEvaluate.end() != it; ++it ) {
            if( !c->ignoreDeps ) {
                _modulesGraphPtr->chain_for_node_by_label( *it, c->units );
            } else {
                // utherwise, just obtain the module pointer:
                c->units.push_front( (*_modulesGraphPtr)( *it ).data_ptr() );
            }
        }
    }
    //if( !c->ignoreDeps ) {  // todo: XXX
    //    c->units.reverse();
    //}

    // erase repititions:
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
UTApp::_run_unit( TestingUnit * unit,
                  std::ostream & os,
                  bool noRun ) {
    ls() << ESC_CLRBOLD << std::setw(48)
         << unit->verbose_name()
         << ESC_CLRCLEAR
         << " ... ";
    unit->make_own_outstream();
    if( co().noCatch ) {
        unit->run_unsafe( noRun );
    } else {
        unit->run( noRun );
    }
    if( 0 == unit->ran_result() ) {
        ls() << ESC_BLDGREEN << "success" << ESC_CLRCLEAR;
    } else if( 2 == unit->ran_result() ) {
        ls() << ESC_BLDYELLOW << "skipped" << ESC_CLRCLEAR;
    } else if( -1 == unit->ran_result() ) {
        ls() << ESC_BLDRED << "failure" << ESC_CLRCLEAR;
    } else if( -2 == unit->ran_result() ) {
        ls() << ESC_BLDRED << "library error" << ESC_CLRCLEAR;
    } else if( -3 == unit->ran_result() ) {
        ls() << ESC_BLDRED << "third-party error" << ESC_CLRCLEAR;
    }
    ls() << std::endl;;
    return unit->ran_result();
}

int
UTApp::_V_run() {
    switch( UTApp::co().operation ) {
        case Config::printBuildConfig : {
            goo_build_info( stdout );
        } break;
        case Config::listUnits : {
            list_modules( std::cout );
        } break;
        case Config::dumpDOT : {
            _modulesGraphPtr->dump_DOT_notated_graph( std::cout );
        } break;
        case Config::runAll :
        case Config::runChoosen : {
            size_t nErrors = 0,
                   nSkipped = 0,
                   nRan = 0;
            Parent::ls() << "Unit tests invokation:" << std::endl;
            for( auto it  = UTApp::co().units.begin();
                 UTApp::co().units.end() != it; ++it ) {
                // to avoid const casting, we provide a search among mutable storage:
                //TestingUnit * mutable // TODO
                int rc = _run_unit( const_cast<TestingUnit *>(*it), Parent::ls() /*, dryRun*/ );
                if( rc < 0 ) { ++nErrors; }
                else if( rc == 0 ) { ++nRan; }
                else if( rc == 2 ) { ++nSkipped; }
            }
            Parent::ls() << "Unit test routine is now finishing up:" << std::endl
                         << " units ran : " << ESC_BLDGREEN << nRan << ESC_CLRCLEAR << std::endl
                         << "   skipped : " << ESC_BLDYELLOW << nSkipped << ESC_CLRCLEAR << std::endl
                         << "    errors : " << (nErrors > 0 ? ESC_BLDRED : ESC_BLDGREEN) << nErrors << ESC_CLRCLEAR << std::endl;
            if( UTApp::co().printUnitsLogs ) {
                Parent::ls() << "REPORTS:" << std::endl;
                for( auto it  = UTApp::co().units.begin();
                     UTApp::co().units.end() != it; ++it ) {
                    if( 2 == (*it)->ran_result() ) {
                        continue;
                    }
                    Parent::ls() << "UNIT ";
                    if( (*it)->ran_result() == 0 ) {
                        Parent::ls() << ESC_BLDGREEN;
                    } else {
                        Parent::ls() << ESC_BLDRED;
                    }
                    Parent::ls() << (*it)->verbose_name() << ESC_CLRCLEAR << ":" << std::endl;
                    Parent::ls() << dynamic_cast<const std::stringstream&>(
                                (*it)->outs()).str() << std::endl;
                }
            }
        } break;
        case Config::unassigned :
        case Config::printHelp :
        default : {
            goo::ut::_print_usage( _appName.c_str() );
        } break;
    };
    return EXIT_SUCCESS;
}

void
UTApp::_incorporate_dependencies() {
    for( auto it  = _modulesStoragePtr->begin();
              it != _modulesStoragePtr->end(); ++it ) {
        auto dependantNode = _modulesGraphPtr->get_node( it->first );
        for( auto depNameIt  = it->second->dep_names().begin();
                  depNameIt != it->second->dep_names().end(); ++depNameIt ) {
            auto depNodeIt = _modulesStoragePtr->find( *depNameIt );
            if( _modulesStoragePtr->end() == depNodeIt ) {
                emraise( noSuchKey, "Unit \"%s\" depends from unknown module: \"%s\".",
                        it->first.c_str(),
                        depNameIt->c_str()
                    );
            }
            auto dependencyNode = _modulesGraphPtr->get_node( *depNameIt );
            dependencyNode.precedes( &dependantNode );
        }
    }
}

//
// Unit
//

void
UTApp::TestingUnit::set_dependencies(
        const char depNames[][48],
        uint8_t nDepNames ) {
    for( uint8_t i = 0; i < nDepNames; ++i ) {
        _depNames.insert( depNames[i] );
    }
}

void
UTApp::TestingUnit::run_unsafe( bool dryRun ) {
    if( dryRun ) {
        _ranResult = 2;
        return;
    }
    _V_run( *_outStream );
    _ranResult = 0;
}

void
UTApp::TestingUnit::run( bool dryRun ) noexcept {
    if( dryRun ) {
        _ranResult = 2;
        return;
    }
    try {
        _V_run( *_outStream );
    } catch( goo::Exception & ge ) {
        ge.dump(outs());
        if( goo::Exception::uTestFailure == ge.code() ) {
            _ranResult = -1;  // Goo's UT expected error
            return;
        } else {
            _ranResult = -2;  // Other (unexpected) Goo's error
            return;
        }
    } catch( std::exception & e ) {
        outs() << "std::exception cought. e.what(): \"" << e.what() << "\"" << std::endl;
        _ranResult = -3;
        return;
    } catch( ... ) {
        _ranResult = -3;  // third party exception
        return;
    }
    _ranResult = 0;
}

void
UTApp::TestingUnit::make_own_outstream() {
    outs(*(new std::stringstream()));
    _outStreamOwn = true;
}

UTApp::TestingUnit::~TestingUnit() {
    if( _outStreamOwn ) {
        delete _outStream;
    }
}

}  // namespace ut
}  // namespace goo

