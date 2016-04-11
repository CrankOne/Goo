# include "goo_dict/dict.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_exception.hpp"
# include "goo_utility.hpp"

# include <cstring>
# include <unistd.h>
# include <getopt.h>
# include <wordexp.h>
# include <list>
# include <cassert>

# if 1

namespace goo {
namespace dict {

Dictionary::Dictionary( const char * name_,
                        const char * description_ ) :
            iAbstractParameter(name_,
                               description_,
                               0 ) {
}

Dictionary::~Dictionary() {
    for( auto it = _parameters.begin();
         it != _parameters.end(); ++it ) {
        delete it->second;
    }
    for( auto it = _byShortcutIndexed.begin();
             it != _byShortcutIndexed.end(); ++it ) {
        delete it->second;
    }
    for( auto it = _dictionaries.begin();
         it != _dictionaries.end(); ++it ) {
        delete it->second;
    }
}

void
Dictionary::insert_parameter( iAbstractParameter * instPtr ) {
    if( instPtr->has_shortcut() ) {
        _byShortcutIndexed.emplace( instPtr->shortcut(),
                                    instPtr );
    } else if( instPtr->name() ) {
        _parameters.emplace( instPtr->name(),
                             instPtr );
    } else {
        emraise( badArchitect, "Got parameter without name and shortcut." );
    }
}

void
Dictionary::insert_section( Dictionary * instPtr ) {
    _dictionaries.emplace( instPtr->name(),
                           instPtr );
}

void
Dictionary::_insert_long_option( const std::string & nameprefix,
                                 Dictionary::LongOptionEntries & q,
                                 const iAbstractParameter & p ) {
    assert( p.name() );
    struct ::option o = {
        strdup((nameprefix + p.name()).c_str()),  // TODO: cleanup
        (p.has_value() ? required_argument : no_argument),
        NULL,
        (p.has_shortcut() ? p.shortcut() : 
            (p.has_value() ? 2 : 1) ) };
    q.push( malloc(sizeof(struct ::option)) );
    memcpy( q.back(), &o, sizeof(o) );
}

void
Dictionary::_append_options( const std::string & nameprefix,
                             ShortOptString & shrtOpts,
                             LongOptionEntries & longOpts ) const {
    //std::cout << "XXX: " << _byShortcutIndexed.size() << ", "  // XXX
    //                     << _parameters.size() << std::endl;   // XXX
    // Form short options string (without any prefixes here):
    for( auto it  = _byShortcutIndexed.cbegin();
              it != _byShortcutIndexed.cend(); ++it ) {
        iAbstractParameter & pRef = *(it->second);
        // It is mandatory for options in this container to
        // have shortcut. It is useful to check that.
        assert( pRef.shortcut() == it->first );
        assert( pRef.has_shortcut() );
        shrtOpts.push( pRef.shortcut() );
        if( pRef.has_value() ) {
            shrtOpts.push( ':' );
        }
        if( pRef.name() ) {
            _insert_long_option( nameprefix, longOpts, pRef );
        }
    }
    // Form long options struct:
    for( auto it  = _parameters.cbegin();
              it != _parameters.cend(); ++it ) {
        iAbstractParameter & pRef = *(it->second);
        // Options at this container must not have
        // shortcuts as they are stored at _byShortcutIndexed
        assert( pRef.name() == it->first );
        assert( ! pRef.has_shortcut() );
        _insert_long_option( nameprefix, longOpts, pRef );
    }
    // Now, recursively traverse via all sub-dictionaries:
    for( auto it  = _dictionaries.cbegin();
              it != _dictionaries.cend(); ++it ) {
        assert( it->second->name() == it->first );
        std::string sectPrefix = nameprefix + it->second->name();
        it->second->_append_options( sectPrefix, shrtOpts, longOpts );
    }
}

// iAbstractParameter interface implementation
//////////////////////////////////////////////

# if 0
const void *
Configuration::_form_long_options() const {
    # if 0
    // ...
    return _cache_longOptionsPtr;
    # else
    struct ::option * lOpts = new struct ::option [7];
    lOpts[0] = {"add",     required_argument, 0,  0 };
    lOpts[1] = {"append",  no_argument,       0,  0 };
    lOpts[2] = {"delete",  required_argument, 0,  0 };
    lOpts[3] = {"verbose", no_argument,       0,  0 };
    lOpts[4] = {"create",  required_argument, 0, 'c'};
    lOpts[5] = {"file",    required_argument, 0,  0 };
    lOpts[6] = {0,         0,                 0,  0 };
    return _cache_longOptionsPtr = lOpts;
    # endif
}

const char *
Configuration::_create_short_opt_string() const {
    # if 0
    // ...
    return _cache_shortOptionsPtr;
    # else
    const char src[] = "-:abc:d:012";
    char * sOpts = new char [ strlen(src) + 1 ];
    strcpy( sOpts, src );
    return _cache_shortOptionsPtr = sOpts;
    # endif
}
# endif

void
Configuration::_recache_getopt_arguments() const {
    _free_caches_if_need();

    Configuration::ShortOptString    sOptsQ;
    Configuration::LongOptionEntries lOptsQ;
    _append_options( "", sOptsQ, lOptsQ );
    static const char _static_shortOptionsPrefix[] = "-:";  // todo: move to define
    const size_t sOptsStrLen = sizeof(_static_shortOptionsPrefix) + sOptsQ.size() - 1;

    _cache_shortOptionsPtr             = new char [ sOptsStrLen + 1 ];
    struct ::option * longOptionsPtr_t = new struct ::option [ lOptsQ.size() + 1 ];
    _cache_longOptionsPtr = longOptionsPtr_t;

    {  // Form short options string:
        memcpy( _cache_shortOptionsPtr, _static_shortOptionsPrefix,
                sizeof(_static_shortOptionsPrefix) );
        for( char * c = _cache_shortOptionsPtr + sizeof(_static_shortOptionsPrefix) - 1;
                        !sOptsQ.empty();
                        ++c, sOptsQ.pop() ) {
            *c = sOptsQ.front();
        }
        _cache_shortOptionsPtr[sOptsStrLen] = '\0';
    }

    {  // Form long options structures array:
        longOptionsPtr_t[lOptsQ.size()] = {NULL, 0, 0, 0};  // sentinel
        for( struct ::option * c = longOptionsPtr_t;
             !lOptsQ.empty();
             ++c, lOptsQ.pop() ) {
            memcpy(c, lOptsQ.front(), sizeof(struct ::option));
        }
    }

    _getoptCachesValid = true;
}

void
Configuration::extract( int argc, char * const argv[], std::ostream * verbose ) {
    # define log_extraction( ... ) if( verbose ) { *verbose << strfmt( __VA_ARGS__ ); }
    ::opterr = 0;  // prevent default `app_name : invalid option -- '%c'' message
    ::optind = 0;  // forses rescan with each parameters vector
    if( !_getoptCachesValid ) {
        _recache_getopt_arguments();
    }
    const struct ::option * longOptions = reinterpret_cast<const ::option *>( _cache_longOptionsPtr );
    if( verbose ) {
        log_extraction( "== Short options string:\n\"%s\"\n"
                        "Long options:\n",
                        _cache_shortOptionsPtr );
        for( const struct ::option * it = longOptions; it->name; ++it ) {
            *verbose << "    -- " << it->name << (required_argument == it->has_arg ? '!' :
                                                 (optional_argument == it->has_arg ? '?' : ' '))
                     << std::endl;
        }
    }
    int optIndex = 0, c;
    while( -1 != (c = getopt_long( argc, argv, _cache_shortOptionsPtr, longOptions, &optIndex )) ) {
        if( isalnum(c) ) {
            // indicates this is an option with shortcut (or a shortcut-only option)
            auto pIt = _shortcuts.find( c );
            if( _shortcuts.end() == pIt ) {
                emraise( badState, "Shortcut option '%c' (0%o) unknown.", c, c );
            }
            if( pIt->second->has_value() ) {
                log_extraction( "c=%c (0%o) considered as a parameter with argument \"%s\".\n",
                                c, c, optarg );
                // TODO ...
            } else {
                log_extraction( "c=%c (0%o) considered as an option.\n", c, c );
                // TODO ...
            }
        } else if( 1 == c ) {
            // Indicates this is a kind of long option (without arg)
            log_extraction( "Got an option \"%s\".\n", longOptions[optind].name );
            // TODO
        } else if( 2 == c ) {
            // Indicates this is a long parameter (with arg)
            log_extraction( "Got parameter \"%s=%s\".\n", longOptions[optind].name, optarg );
            // TODO
        } else if( '?' == c ) {
            emraise( badParameter, "Option '%c' (0%o) unrecognized.", optopt, (int) optopt );
        } else if( ':' == c ) {
            // todo: any additional info?
            emraise( argumentExpected, "Parametered option expects an argument." );
        } else if(  01 == c ) {
            // this is a positional argument
            log_extraction( "\"%s\" considered as a positional argument.\n", ::optarg );
            // TODO ...
        } else {
            emraise( badValue, "getopt() returned character code 0%o.", c );
        }
    }
    // TODO: further processing of positional arguments.
    # undef log_extraction
}

InsertionProxy
Configuration::insertion_proxy() {
    return InsertionProxy( this );
}

Configuration::Configuration( const char * name_,
                              const char * descr_ ) : Dictionary(name_, descr_),
                                                      _cache_longOptionsPtr( nullptr ),
                                                      _cache_shortOptionsPtr( nullptr ),
                                                      _getoptCachesValid( false ) {
}

void
Configuration::_free_caches_if_need() const {
    if( _cache_longOptionsPtr ) {
        delete [] reinterpret_cast<::option *>(_cache_longOptionsPtr);
    }
    if( _cache_shortOptionsPtr ) {
        delete [] _cache_shortOptionsPtr;
    }
}

Configuration::~Configuration() {
    _free_caches_if_need();
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
Configuration::insert_parameter( iAbstractParameter * p_ ) {
    _getoptCachesValid = false;
    Dictionary::insert_parameter( p_ );
    if( p_->has_shortcut() ) {
        auto it = _shortcuts.find( p_->shortcut() );
        if( it != _shortcuts.end() ) {
            emraise( nonUniq, "Configuration \"%s\":%p already has shortened option '-%c'.",
                    this->name(), this,
                    p_->shortcut() );
        }
        _shortcuts.emplace( p_->shortcut(), p_ );
    }
    // TODO: _longOptions
}

void
Configuration::insert_section( Dictionary * sect ) {
    _getoptCachesValid = false;
    Dictionary::insert_section( sect );
}

void
Configuration::_insert_shortened_parameter( iAbstractParameter * ) {
}

void
Configuration::_insert_fully_qualified_parameter( const std::string &, iAbstractParameter * ) {
}

}  // namespace dict
}  // namespace dict

# endif

