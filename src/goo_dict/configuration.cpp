/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# include "goo_dict/configuration.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_exception.hpp"
# include "goo_utility.hpp"

# include <cstring>
# include <unistd.h>
# include <getopt.h>
# include <wordexp.h>
# include <list>
# include <cassert>
# include <sstream>
# include <climits>

namespace goo {
namespace dict {

const int Configuration::longOptKey = 2;
const int Configuration::longOptNoShortcutRequiresArgument = 3;

Configuration::Configuration( const char * name_,
                              const char * descr_ ) : Dictionary(name_, descr_),
                                                      _cache_longOptionsPtr( nullptr ),
                                                      _cache_shortOptionsPtr( nullptr ),
                                                      _getoptCachesValid( false ) {
}

Configuration::Configuration( const Configuration & orig ) : Dictionary( orig ),
                                                      _cache_longOptionsPtr( nullptr ),
                                                      _cache_shortOptionsPtr( nullptr ),
                                                      _getoptCachesValid( false ) {}

Configuration::~Configuration() {
    _free_caches_if_need();
}

void
Configuration::_recache_getopt_arguments() const {
    _free_caches_if_need();

    Configuration::ShortOptString    sOptsQ;
    Configuration::LongOptionEntries lOptsQ;
    _cache_append_options( *this, "", sOptsQ, _cache_shortcutPaths, lOptsQ );
    static const char _static_shortOptionsPrefix[] = "-:";  // todo: move to define
    const size_t sOptsStrLen = sizeof(_static_shortOptionsPrefix) + sOptsQ.size() - 1;

    // TODO: string with back-insertion iterator?
    _cache_shortOptionsPtr             = new char [ sOptsStrLen + 1 ];
    struct ::option * longOptionsPtr_t = new struct ::option [ lOptsQ.size() + 1 ];
    _cache_longOptionsPtr = longOptionsPtr_t;

    {  // Form short options string:
        memcpy( _cache_shortOptionsPtr, _static_shortOptionsPrefix,
                sizeof(_static_shortOptionsPrefix) );
        for( char * c = _cache_shortOptionsPtr + sizeof(_static_shortOptionsPrefix) - 1;
                        !sOptsQ.empty();
                        ++c, sOptsQ.pop_front() ) {
            *c = sOptsQ.front();
        }
        _cache_shortOptionsPtr[sOptsStrLen] = '\0';
    }

    {  // Form long options structures array:
        longOptionsPtr_t[lOptsQ.size()] = {NULL, 0, 0, 0};  // sentinel
        struct ::option * queuedInstancePtr;
        for( struct ::option * c = longOptionsPtr_t;
             !lOptsQ.empty();
             ++c, lOptsQ.pop_front() ) {
            memcpy(c,
                   queuedInstancePtr = reinterpret_cast<struct ::option *>(lOptsQ.front()),
                   sizeof(struct ::option) );
            free( queuedInstancePtr );
        }
    }
    _getoptCachesValid = true;
}

void
Configuration::_set_argument_parameter( iSingularParameter & p,
                                        const char * strval,
                                        std::ostream * verbose ) {
    if( p.is_singular() ) {
        iSingularParameter & sp = auth_cast<iSingularParameter&>(p);
        sp.parse_argument( strval );
        if( verbose ) {
            *verbose << strfmt( "    ...set to \"%s\".", sp.to_string().c_str() )
                     << std::endl;
        }
    } else {
        iSingularParameter & mp = auth_cast<iSingularParameter&>(p);
        mp.parse_argument( strval );
        if( verbose ) {
           *verbose << strfmt( "    ...appended with \"%s\".", mp.to_string().c_str() )
                    << std::endl;
        }
        // or whatever ...
    }
}

/** Auxiliary routine filling structure for further getopt_long() invokation.
 * Ready `::option` structure (from C stdlib) will be composed accordingly to
 * following rules:
 *      - `name` field will consist of nameprefix + parameter name
 *      - `has_arg` will be set to 1 if parameter requires an argument or is a
 *        boolean option.
 *      - `flag` will be set to NULL
 *      - `val` will be set to shortcut if it was associated with parameter.
 *        Otherwise , value will be set to `longOptNoShortcutRequiresArgument`
 *        or `longOptKey` depending of whether the parameter requies an
 *        argument or not correspondingly.
 * The resulting structure may further be used for parsing command-line
 * arguments and will be appended to the back of given long options list.
 * */
void
Configuration::_cache_insert_long_option( const std::string & nameprefix,
                                          Dictionary::LongOptionEntries & q,
                                          const iSingularParameter & p ) {
    assert( p.name() );
    struct ::option o = {
        strdup((nameprefix + p.name()).c_str()),
        (p.requires_value() ? required_argument :
                        ( dynamic_cast<const Parameter<bool>*>(&p) ?
                                            optional_argument : no_argument ) ),
        NULL,
        (p.has_shortcut() ? p.shortcut() :
                (p.requires_value() ? longOptNoShortcutRequiresArgument
                                    : longOptKey) ) };
    q.push_back( malloc(sizeof(struct ::option)) );
    memcpy( q.back(), &o, sizeof(o) );
}

void
Configuration::_cache_append_options( const Dictionary & self,
                                      const std::string & nameprefix,
                                      ShortOptString & shrtOpts,
                                      std::unordered_map<char, std::string> & shrtPths,
                                      LongOptionEntries & longOpts ) {
    // Form short options string (without any prefixes here):
    for( auto it  = self._parametersIndexByShortcut.cbegin();
              it != self._parametersIndexByShortcut.cend(); ++it ) {
        iSingularParameter & pRef = *(it->second);
        // It is mandatory for parameters in this index to
        // have shortcut. It is handy to check their shortcuts here to catch
        // any possible mistakes due to bad insertion procedures.
        assert( pRef.shortcut() == it->first );
        assert( pRef.has_shortcut() );
        // 
        shrtOpts.push_back( pRef.shortcut() );
        if( pRef.requires_value() ) {
            shrtOpts.push_back( ':' );
        }
        auto ir = shrtPths.emplace( pRef.shortcut(), nameprefix );
        if( !ir.second ) {
            emraise( nonUniq, "Shortcut option '%c' had been already inserted "
                "into Configuration instance within parameter entry "
                "by path %s while insertion of parameter entry "
                "by path %s requested with the same single-char "
                "shortcut.",
                pRef.shortcut(), ir.first->second.c_str(), nameprefix.c_str() );
        }
    }
    // Form long-only options struct:
    for( auto it  = self._parametersIndexByName.cbegin();
              it != self._parametersIndexByName.cend(); ++it ) {
        iSingularParameter & pRef = *(it->second);
        assert( pRef.name() == it->first );
        _cache_insert_long_option( nameprefix, longOpts, pRef );
    }
    // Now, recursively traverse via all sub-dictionaries (subsections):
    for( auto it  = self._dictionaries.cbegin();
              it != self._dictionaries.cend(); ++it ) {
        assert( it->second->name() == it->first );
        std::string sectPrefix = nameprefix + it->second->name() + ".";
        _cache_append_options( *(it->second), sectPrefix,
                                                shrtOpts, shrtPths, longOpts );
    }
}

void
Configuration::extract( int argc,
                        char * const argv[],
                        bool doConsistencyCheck,
                        std::ostream * verbose ) {
    # define log_extraction( ... ) if( verbose ) { *verbose << strfmt( __VA_ARGS__ ); }
    ::opterr = 0;  // prevent default `app_name : invalid option -- '%c'' message
    ::optind = 0;  // forses rescan with each parameters vector
    if( !_getoptCachesValid ) {
        _recache_getopt_arguments();
    }
    assert( _cache_longOptionsPtr );
    const struct ::option * longOptions = reinterpret_cast<const ::option *>( _cache_longOptionsPtr );
    if( verbose ) {
        log_extraction( "parserCaches:\n    shortOptions: \"%s\"\n"
                        "    longOptions:\n",
                        _cache_shortOptionsPtr );
        size_t n = 0;
        for( const struct ::option * it = longOptions; it->name; ++it, ++n ) {
            *verbose << std::string(8, ' ') << it->name << ":" << std::endl;
            *verbose << std::string(12, ' ') << "argument: \""
                     << (required_argument == it->val ? "required" :
                        (optional_argument == it->val ? "optional" : "<none-or-shortcut>") )
                     << "\"" << std::endl
                     << std::string(12, ' ') << "getoptVal: "
                     << std::hex << std::showbase << it->val
                     << std::endl;
        }
    }
    // TODO: check whether getopt_long arguments (composed caches) are empty.
    // If they are, RAISE EXCEPTION.
    int optIndex = -1, c;
    while( -1 != (c = getopt_long( argc, argv, _cache_shortOptionsPtr, longOptions, &optIndex )) ) {
        //if ( optind == prevInd + 2 && *optarg == '-' ) {
        //    c = ':';
        //    -- optind;
        //}
        if( isalnum(c) ) {
            // indicates this is an option with shortcut (or a shortcut-only option)
            auto pIt = _parametersIndexByShortcut.find( c );
            if( _parametersIndexByShortcut.end() == pIt ) {
                auto byPath = _cache_shortcutPaths.find( c );
                if( _cache_shortcutPaths.end() == byPath ) {
                    emraise( badState, "Shortcut option '%c' (0x%02x) unknown.",
                         c, c );
                }
                // The parameter is referred by shortcut and lies in
                // one of the sub-sections referred by path:
                Dictionary & subsection = this->subsection( byPath->second.c_str() );
                pIt = subsection._parametersIndexByShortcut.find( c );
                assert( subsection._parametersIndexByShortcut.end() != pIt );  // impossible by design
            }
            iSingularParameter & parameter = *(pIt->second);
            if( parameter.requires_value() ) {
                log_extraction( "c=%c (0x%02x) considered as a (short) "
                                "parameter with argument \"%s\".\n", c, c, optarg );
                if( strnlen(optarg, USHRT_MAX) > 1
                    && '-' == optarg[0]
                    && (!isdigit(optarg[1])) ) {
                    // This is, apparently, another option, not an argument. Note,
                    // that dash symbol alone (as an argv token) is allowed as it
                    // usually refers to stdout/stdin. Also we have to support
                    // negative numbers.
                    emraise( badState, "Option \"%c\" requires an argument, but "
                        "next command-line argument seems to be another "
                        "option: \"%s\".", c, optarg);
                }
                _set_argument_parameter( parameter, optarg, verbose );
            } else {
                log_extraction( "c=%c (0x%02x) considered as an option.\n", c, c );
                try {
                    dynamic_cast<Parameter<bool>&>(parameter).set_option(true);
                    log_extraction( "    ...c=%c (0x%02x) has been "
                                    "set/appended with (=True).\n", c, c );
                } catch( std::bad_cast & e ) {
                    emraise( badCast, "Parameter '%c' can not be "
                            "considered as an option and requires an "
                            "argument.", c );
                }
            }
        } else if( longOptKey == c ) {
            if( -1 == optIndex ) {
                emraise( badState, "`getopt_long()` parser falled to wrong "
                    "state: long option without an argument suggested, but "
                    "long option index is unset.");
            }
            // Indicates this is a kind of long option (logical parameter
            // without argument).
            assert( longOptions[optIndex].name );
            log_extraction( "getopt_long() returned %d integer value --- "
                            "considering option \"%s\" (optIndex=%d).\n", c,
                            longOptions[optIndex].name, optIndex );
            auto pIt = _parametersIndexByName.find( longOptions[optIndex].name );
            if( _parametersIndexByName.end() == pIt ) {
                
            }
            iSingularParameter & p = Dictionary::parameter( 
                                                longOptions[optIndex].name );
            if( p.requires_value() ) {
                // If went here, the getopt_long() returned a value indicating
                // the long parameter without an argument, but obtained
                // parameter instance does require an argument. It does indicate
                // a bug.
                emraise( badState, "Parameter \"%s\" can not be considered as a "
                        "logic option; it does require an argument.",
                        longOptions[optIndex].name );
            }
            try {
                dynamic_cast<Parameter<bool>&>(p).set_option(true);
                log_extraction( "    ...\"%s\" has been set/appended with "
                                "(=True).\n", longOptions[optIndex].name );
            } catch( std::bad_cast & e ) {
                emraise( badCast, "Parameter \"%s\" can not be considered as a "
                        "logic option.",
                        longOptions[optIndex].name );
            }
        } else if( longOptNoShortcutRequiresArgument == c ) {
            if( -1 == optIndex ) {
                emraise( badState, "`getopt_long()` parser falled to wrong "
                    "state: long option with required argument suggested, but "
                    "long option index is unset.");
            }
            // Indicates this is a long parameter (with arg)
            assert( longOptions[optIndex].name );
            log_extraction( "\"%s=%s\" considered as a (long) parameter.\n",
                            longOptions[optIndex].name, optarg );
            if( strnlen(optarg, USHRT_MAX) > 1
                && '-' == optarg[0]
                && (!isdigit(optarg[1])) ) {
                // This is, apparently, another option, not an argument. Note,
                // that dash symbol alone (as an argv token) is allowed as it
                // usually refers to stdout/stdin. Also we have to support
                // negative numbers.
                emraise( badState, "Parameter \"%s\" requires an argument, but "
                    "next command-line argument seems to be another option: "
                    "\"%s\".", longOptions[optIndex].name, optarg);
            }
            _set_argument_parameter(
                    Dictionary::parameter(  longOptions[optIndex].name ),
                                            optarg,
                                            verbose );
        } else if( '?' == c ) {
            if( optopt ) {
                emraise( parserFailure, "Command-line argument is not "
                    "recognized (charcode %#02x, '%c').", (int) optopt, optopt );
            } else {
                emraise( parserFailure, "Command-line argument is not "
                    "recognized (charcode %#02x). Suspicious token: \"%s\".",
                    (int) optopt, argv[optind-1] );
            }
        } else if( ':' == c ) {
            // todo: any additional info?
            emraise( argumentExpected, "Parametered option expects an argument." );
        } else if(  01 == c ) {
            // this is a positional argument (because we have set the '-'
            // character first in optstring).
            log_extraction( "\"%s\" considered as a positional argument.\n", ::optarg );
            _TODO_  // TODO ...
        } else {
            emraise( badValue, "getopt() returned character code %#02x.", c );
        }
        optIndex = -1;
    }
    // TODO: further processing of positional arguments: forward ones given
    // after "--" token.
    {
        std::map<std::string, const iSingularParameter *> badParameters;
        if( doConsistencyCheck && !this->is_consistant( badParameters, "" ) ) {
            emraise( inconsistentConfig,
                    "Some required arguments aren't set." );
        }
    }
    # undef log_extraction
}

InsertionProxy
Configuration::insertion_proxy() {
    return InsertionProxy( this );
}

void
Configuration::_free_caches_if_need() const {
    if( _cache_longOptionsPtr ) {
        for( struct ::option * c = reinterpret_cast<struct ::option *>(_cache_longOptionsPtr);
             c->name || c->has_arg || c->flag || c->val; ++c ) {
            if( c->name ) {
                free( const_cast<char*>(c->name) );
            }
        }
        delete [] reinterpret_cast<::option *>(_cache_longOptionsPtr);
        _cache_longOptionsPtr = nullptr;
    }
    if( _cache_shortOptionsPtr ) {
        delete [] _cache_shortOptionsPtr;
        _cache_shortOptionsPtr = nullptr;
    }
    _cache_shortcutPaths.clear();
    _getoptCachesValid = false;
}

void
Configuration::usage_text( std::ostream & os,
                           bool enableASCIIColoring ) {
    os << name() << " --- " << description() << std::endl
       << "Usage:" << std::endl;
}

Size
Configuration::tokenize_string( const std::string & str, char **& argvTokens ) {
    ::wordexp_t pwords;
    int rc = wordexp( str.c_str(), &pwords, WRDE_UNDEF | WRDE_SHOWERR | WRDE_NOCMD );
    if( rc ) {
        if( WRDE_BADCHAR == rc) {
            emraise( badParameter, "Bad character met." );
        } else if( WRDE_BADVAL == rc ) {
            emraise( badArchitect, "Reference to undefined shell variable met." );
        } else if( WRDE_CMDSUB == rc ) {
            emraise( badState, "Command substitution requested." );
        } else if( WRDE_NOSPACE == rc ) {
            emraise( memAllocError, "Attempt to allocate memory failed." );
        } else if( WRDE_SYNTAX == rc ) {
            emraise( interpreter, "Shell syntax error." );
        }
    }
    const Size nWords = pwords.we_wordc;
    argvTokens = (char**) malloc( sizeof(char*)*nWords );
    for( size_t n = 0; n < nWords; ++n ) {
        argvTokens[n] = strdup( pwords.we_wordv[n] );
    }
    wordfree( &pwords );
    return nWords;
}

void
Configuration::free_tokens( size_t argcTokens, char ** argvTokens ) {
    for( size_t n = 0; n < argcTokens; ++n ) {
        free( argvTokens[n] );
    }
    free( argvTokens );
}

void
Configuration::insert_parameter( iSingularParameter * p_ ) {
    Dictionary::insert_parameter( p_ );
    invalidate_getopt_caches();
}

void
Configuration::insert_section( Dictionary * sect ) {
    Dictionary::insert_section( sect );
    invalidate_getopt_caches();
}

# if 0
void
Configuration::_append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const {
    for( auto it  = _dictionaries.cbegin();
              it != _dictionaries.cend(); ++it ) {
        it->second->_append_configuration_caches(
                it->first, conf
            );
    }
}
# endif

void
Configuration::_cache_parameter_by_shortcut( iSingularParameter * p_ ) {
    assert( p_->has_shortcut() );
    // TODO: insert and check insertion result
    auto it = _parametersIndexByShortcut.find( p_->shortcut() );
    if( it != _parametersIndexByShortcut.end() ) {
        emraise( nonUniq, "Configuration \"%s\":%p already has shortened option '-%c'.",
                this->name(), this,
                p_->shortcut() );
    }
    _parametersIndexByShortcut.emplace( p_->shortcut(), p_ );
}

void
Configuration::_cache_parameter_by_full_name( const std::string & fullName,
                                              iSingularParameter * p_ ) {
    assert( !p_->has_shortcut() );
    assert( p_->name() );
    // TODO: insert and check insertion result
    auto it = _parametersIndexByName.find( fullName );
    if( it != _parametersIndexByName.end() ) {
        emraise( nonUniq, "Configuration \"%s\":%p already has option '--%s'.",
                this->name(), this,
                fullName.c_str() );
    }
    _parametersIndexByName.emplace( fullName.c_str(), p_ );
}

void
Configuration::print_ASCII_tree( std::ostream & ss ) const {
    std::list<std::string> fullOutput;
    print_ASCII_tree( fullOutput );
    for( auto line : fullOutput ) {
        ss << line << std::endl;
    }
}

void
Configuration::print_ASCII_tree( std::list<std::string> & output ) const {
    Dictionary::print_ASCII_tree( output );
    if( output.empty() ) {
        output.push_back( std::string("╚═ " ESC_BLDRED "<!empty!>" ESC_CLRCLEAR) );
    }
    output.push_front( std::string("╔═ << " ESC_BLDGREEN) + name() + ESC_CLRCLEAR " >>" );
}

}  // namespace dict
}  // namespace goo

