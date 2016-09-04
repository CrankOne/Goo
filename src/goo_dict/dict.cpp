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
# include <sstream>

# if 1

namespace goo {
namespace dict {

Dictionary::Dictionary( const char * name_,
                        const char * description_ ) :
            DuplicableParent(name_,
                             description_,
                             0 ) {
}

Dictionary::Dictionary( const Dictionary & orig ) : DuplicableParent( orig ) {
    for( auto it  = orig._parameters.begin();
              it != orig._parameters.end(); ++it ) {
        //_parameters.insert(
        //        DECLTYPE(_parameters)::value_type(
        //                    it->first,
        //                    clone_as<iAbstractParameter, iSingularParameter>( it->second ) )
        //    );
        if( it->second.second ) {
            insert_parameter( clone_as<iAbstractParameter, iSingularParameter>( it->second.first ) );
        }
    }
    for( auto it = orig._byShortcutIndexed.begin();
             it != orig._byShortcutIndexed.end(); ++it ) {
        //_byShortcutIndexed.insert(
        //        DECLTYPE(_byShortcutIndexed)::value_type(
        //                    it->first,
        //                    clone_as<iAbstractParameter, iSingularParameter>( it->second ) )
        //    );
        insert_parameter( clone_as<iAbstractParameter, iSingularParameter>( it->second ) );
    }
    for( auto it  = orig._dictionaries.begin();
              it != orig._dictionaries.end(); ++it ) {
        //_dictionaries.insert(
        //        DECLTYPE(_dictionaries)::value_type(
        //                    it->first,
        //                    clone_as<iAbstractParameter, Dictionary>( it->second ) )
        //    );
        insert_section( clone_as<iAbstractParameter, Dictionary>( it->second ) );
    }
}

Dictionary::~Dictionary() {
    for( auto it = _parameters.begin();
         it != _parameters.end(); ++it ) {
        if( it->second.second ) {
            delete it->second.first;
        }
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
Dictionary::insert_parameter( iSingularParameter * instPtr ) {
    if( instPtr->has_shortcut() ) {
        _byShortcutIndexed.emplace( instPtr->shortcut(),
                                    instPtr );
        if( instPtr->name() ) {
            _parameters.insert( DECLTYPE(_parameters)::value_type( instPtr->name(),
                                 std::pair<iSingularParameter *, bool>(instPtr, false) ) );
        }
    } else if( instPtr->name() ) {
        _parameters.insert( DECLTYPE(_parameters)::value_type( instPtr->name(),
                             std::pair<iSingularParameter *, bool>(instPtr, true ) ) );
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
                                 const iSingularParameter & p ) {
    assert( p.name() );
    struct ::option o = {
        strdup((nameprefix + p.name()).c_str()),
        (p.requires_value() ? required_argument : ( dynamic_cast<const Parameter<bool>*>(&p) ?
                                                        optional_argument : no_argument ) ),
        NULL,
        (p.has_shortcut() ? p.shortcut() : 
            (p.requires_value() ? 2 : 1) ) };
    q.push( malloc(sizeof(struct ::option)) );
    memcpy( q.back(), &o, sizeof(o) );
}

void
Dictionary::_append_options( const std::string & nameprefix,
                             ShortOptString & shrtOpts,
                             LongOptionEntries & longOpts ) const {
    // Form short options string (without any prefixes here):
    for( auto it  = _byShortcutIndexed.cbegin();
              it != _byShortcutIndexed.cend(); ++it ) {
        iSingularParameter & pRef = *(it->second);
        // It is mandatory for options in this container to
        // have shortcut. It is useful to check that.
        assert( pRef.shortcut() == it->first );
        assert( pRef.has_shortcut() );
        shrtOpts.push( pRef.shortcut() );
        if( pRef.requires_value() ) {
            shrtOpts.push( ':' );
        }
        if( pRef.name() ) {
            _insert_long_option( nameprefix, longOpts, pRef );
        }
    }
    // Form long options struct:
    for( auto it  = _parameters.cbegin();
              it != _parameters.cend(); ++it ) {
        iSingularParameter & pRef = *(it->second.first);
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

void
Dictionary::_append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const {
    // Note, that nameprefix already contains this section name.
    for( auto it  = _byShortcutIndexed.cbegin();
              it != _byShortcutIndexed.cend(); ++it ) {
        conf->_cache_parameter_by_shortcut( it->second );
    }
    for( auto it  = _parameters.cbegin();
              it != _parameters.cend(); ++it ) {
        conf->_cache_parameter_by_full_name( nameprefix + "." + it->first, it->second.first );
    }
    for( auto it  = _dictionaries.cbegin();
              it != _dictionaries.cend(); ++it ) {
        it->second->_append_configuration_caches(
                nameprefix + "." + it->first, conf
            );
    }
}

int
Dictionary::pull_opt_path_token( char *& path,
                                 char *& current ) {
    current = path;
    for( ; *path != '\0'; ++path ) {
        if( '.' == *path ) {
            *path = '\0';
            ++path;
            break;
        }
        if( !isalnum(*path) && '-' != *path ) {
            emraise( badParameter,
                     "Path specification contains unallowed character 0x%x.",
                     *path );
        }
    }
    if( '\0' == *path ) {
        return 0;  // no more tokens
    }
    return 1;  // tail is not empty
}

const iSingularParameter &
Dictionary::_get_parameter( char path[] ) const {
    char * current;
    int rc = pull_opt_path_token( path, current );
    if( 0 == rc ) {
        // terminal case --- consider the `current' indexes
        // an option and acquire the value.
        if( path - current > 1 ) {
            // option parameter indexed by long name
            auto it = _parameters.find( current );
            if( it == _parameters.end() ) {
                emraise( notFound,
                     "Option \"%s\" (long name considered) not found in section \"%s\"",
                     current, name() ? name() : "<root>" );
            }
            return *(it->second.first);
        } else if( path - current == 0 ) {
            emraise( badState, "Unexpected state of option path parser --- null option length."
                               "See sources for details." );
        } else {
            // option parameter indexed by shortcut
            auto it = _byShortcutIndexed.find( *current );
            if( it == _byShortcutIndexed.end() ) {
                emraise( notFound,
                     "Option \"%s\" (shortcut considered) not found in section \"%s\"",
                     current, name() ? name() : "<root>" );
            }
            return *(it->second);
        }
    } else {
        // proceed recursively within section -- `current' contains
        // section name and the `path' leads to an option.
        auto it = _dictionaries.find( current );
        if( it == _dictionaries.end() ) {
            emraise( notFound,
                     "Section \"%s\" not found in section \"%s\"",
                     current, name() ? name() : "<root>" );
        }
        return it->second->_get_parameter( path );
    }
}

bool
Dictionary::is_consistant( std::map<std::string, const iSingularParameter *> & badParameters,
                           const std::string & prefix ) const {
    for( auto it  = _parameters.cbegin();
              it != _parameters.cbegin(); ++it ) {
        if( it->second.first->is_mandatory() && !it->second.first->is_set() ) {
            badParameters.emplace(
                    prefix + it->second.first->name(),
                    it->second.first
                );
        }
    }
    for( auto it  = _byShortcutIndexed.cbegin();
              it != _byShortcutIndexed.cbegin(); ++it ) {
        if( it->second->is_mandatory() && !it->second->is_set() ) {
            if( it->second->name() ) {
                badParameters.emplace(
                        prefix + it->second->name(),
                        it->second
                    );
            } else {
                char bf[2] = {it->first, '\0'};
                badParameters.emplace(
                    bf,
                    it->second
                );
            }
        }
    }
    for( auto it  = _dictionaries.cbegin();
              it != _dictionaries.cbegin(); ++it ) {
        it->second->is_consistant( badParameters, prefix + it->first );
    }
    return badParameters.empty();
}

void
Dictionary::print_ASCII_tree( std::list<std::string> & output ) const {
    std::stringstream ss;
    size_t n = _parameters.size();
    bool hasShortcuts = _byShortcutIndexed.size();
    for( auto lp : _parameters ) {
        n--;
        ss << (n || hasShortcuts ? "╟─" : "╙─") << " ";
        if( lp.second.first->name() ) {
            ss << "--" << lp.second.first->name();
            if( lp.second.first->has_shortcut() ) {
                ss << ",";
            }
        }
        if( lp.second.first->has_shortcut() ) {
            ss << "-" << lp.second.first->shortcut();
        }
        // TODO: other properties
        ss << std::endl;
    }
    n = _byShortcutIndexed.size();
    for( auto lp : _byShortcutIndexed ) {
        n--;
        ss << (n ? "╟─" : "╙─") << " ";
        if( lp.second->name() ) {
            ss << "--" << lp.second->name();
            if( lp.second->has_shortcut() ) {
                ss << ",";
            }
        }
        if( lp.second->has_shortcut() ) {
            ss << "-" << lp.second->shortcut();
        }
        // TODO: other properties
        ss << std::endl;
    }
    n = _dictionaries.size();
    for( auto dctPair : _dictionaries ) {
        n--;
        std::list<std::string> sub;
        dctPair.second->print_ASCII_tree( sub );
        ss << " " << (n ? "╠═" : "╚═") << " < " ESC_CLRGREEN << dctPair.first << ESC_CLRCLEAR " > " << std::endl;
        for( auto line : sub ) {
            ss << (n ? "║ " : "  ") << line;
        }
    }

    std::string line;
    while( std::getline(ss, line) ) {
        output.push_back( line );
    }
}


// Configuration
///////////////

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
        struct ::option * queuedInstancePtr;
        for( struct ::option * c = longOptionsPtr_t;
             !lOptsQ.empty();
             ++c, lOptsQ.pop() ) {
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
            *verbose << strfmt( "    ...set to \"%s\".", sp.to_string().c_str() ) << std::endl;
        }
    } else {
        iSingularParameter & mp = auth_cast<iSingularParameter&>(p);
        mp.parse_argument( strval );
        if( verbose ) {
           *verbose << strfmt( "    ...appended with \"%s\".", mp.to_string().c_str() ) << std::endl;
        }
        // or whatever ...
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
        log_extraction( "== Short options string:\n\"%s\"\n"
                        "Long options:\n",
                        _cache_shortOptionsPtr );
        size_t n = 0;
        for( const struct ::option * it = longOptions; it->name; ++it, ++n ) {
            *verbose << "    -- [" << n << "]"
                     << it->name << (required_argument == it->has_arg ? '!' :
                                    (optional_argument == it->has_arg ? '?' : ' ') )
                     << " 0x" << std::hex << it->val
                     << std::endl;
        }
    }
    // TODO: check whether getopt_long arguments (composed caches) are empty.
    // If they are, RAISE EXCEPTION.
    int optIndex = 0, c;
    while( -1 != (c = getopt_long( argc, argv, _cache_shortOptionsPtr, longOptions, &optIndex )) ) {
        if( isalnum(c) ) {
            // indicates this is an option with shortcut (or a shortcut-only option)
            auto pIt = _shortcuts.find( c );
            if( _shortcuts.end() == pIt ) {  // impossibru!
                emraise( badState, "Shortcut option '%c' (0x%02x) unknown.", c, c );
            }
            iSingularParameter & parameter = *(pIt->second);
            if( parameter.requires_value() ) {
                log_extraction( "c=%c (0x%02x) considered as a parameter with argument \"%s\".\n",
                                c, c, optarg );
                _set_argument_parameter( parameter, optarg, verbose );
            } else {
                log_extraction( "c=%c (0x%02x) considered as an option.\n", c, c );
                try {
                    dynamic_cast<Parameter<bool>&>(parameter).set_option(true);
                    log_extraction( "    ...c=%c (0x%02x) has been set/appended with (=True).\n", c, c );
                } catch( std::bad_cast & e ) {
                    emraise( badCast, "Option '%c' can not be considered as option and requires an argument.",
                             c );
                }
            }
        } else if( 1 == c ) {
            // Indicates this is a kind of long option (without arg)
            assert( longOptions[optIndex].name );
            log_extraction( "\"%s\" considered as an option.\n", longOptions[optIndex].name );
            auto pIt = _longOptions.find( longOptions[optIndex].name );
            if( _longOptions.end() == pIt ) {  // impossibru!
                log_extraction( "Available options in long options cache:\n" );
                if( _longOptions.empty() ) {
                    log_extraction( "  <no long options in cache>\n" );
                }
                for( auto p : _longOptions ) {
                    log_extraction( "  --%s\n", p.first.c_str() );
                }
                emraise( badState, "Option \"%s\" not found in caches!", longOptions[optIndex].name );
            }
            iSingularParameter & parameter = *(pIt->second);
            try {
                dynamic_cast<Parameter<bool>&>(parameter).set_option(true);
                log_extraction( "    ...\"%s\" has been set/appended with (=True).\n", longOptions[optIndex].name );
            } catch( std::bad_cast & e ) {
                emraise( badCast, "Option '%c' can not be considered as option and requires an argument.",
                         c );
            }
        } else if( 2 == c ) {
            // Indicates this is a long parameter (with arg)
            assert( longOptions[optIndex].name );
            log_extraction( "\"%s=%s\" considered as a parameter.\n", longOptions[optIndex].name, optarg );
            auto pIt = _longOptions.find( longOptions[optIndex].name );
            if( _longOptions.end() == pIt ) {  // impossibru!
                emraise( badState, "Option \"%s\" not found in caches.", longOptions[optIndex].name );
            }
            iSingularParameter & parameter = *(pIt->second);
            _set_argument_parameter( parameter, optarg, verbose );
        } else if( '?' == c ) {
            emraise( badParameter, "Option '%c' (0x%02x) unrecognized.", optopt, (int) optopt );
        } else if( ':' == c ) {
            // todo: any additional info?
            emraise( argumentExpected, "Parametered option expects an argument." );
        } else if(  01 == c ) {
            // this is a positional argument
            log_extraction( "\"%s\" considered as a positional argument.\n", ::optarg );
            // TODO ...
        } else {
            emraise( badValue, "getopt() returned character code 0x%02x.", c );
        }
    }
    // TODO: further processing of positional arguments.
    {
        std::map<std::string, const iSingularParameter *> badParameters;
        if( doConsistencyCheck && !this->is_consistant( badParameters, "" ) ) {
            emraise( badParameter, "Some required arguments aren't set." );
        }
    }
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

Configuration::Configuration( const Configuration & orig ) : Dictionary( orig ),
                                                      _cache_longOptionsPtr( nullptr ),
                                                      _cache_shortOptionsPtr( nullptr ),
                                                      _getoptCachesValid( false ) {
    _getoptCachesValid = false;
    # if 0
    for( auto shrtPair : orig._shortcuts ) {
        char pathSpec[] = { shrtPair.first };
        const iSingularParameter & myParRef = orig._get_parameter( pathSpec );
        _shortcuts.emplace( shrtPair.first, &const_cast<iSingularParameter &>(myParRef) );
        //std::cout << "XXX: copying shrtct " << shrtPair.first << std::endl;  // XXX
    }
    for( auto shrtPair : orig._longOptions ) {
        char * pathSpec = strdup( shrtPair.first.c_str() );
        const iSingularParameter & myParRef = orig._get_parameter( pathSpec );
        _longOptions.emplace( shrtPair.first, &const_cast<iSingularParameter &>(myParRef) );
        //std::cout << "XXX: copying long opt " << pathSpec << std::endl;  // XXX
        free( pathSpec );
    }
    # endif
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
    _getoptCachesValid = false;
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
Configuration::insert_parameter( iSingularParameter * p_ ) {
    _getoptCachesValid = false;
    Dictionary::insert_parameter( p_ );
    if( p_->has_shortcut() ) {
        _cache_parameter_by_shortcut( p_ );
    } else if( p_->name()  ) {
        _cache_parameter_by_full_name( p_->name(), p_ );
    } else {
        emraise( malformedArguments,
                 "Parameter option %p has not full name, nor shortcut char.",
                 p_ );
    }
}

void
Configuration::insert_section( Dictionary * sect ) {
    _getoptCachesValid = false;
    Dictionary::insert_section( sect );
    _append_configuration_caches( "", this );
}

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

void
Configuration::_cache_parameter_by_shortcut( iSingularParameter * p_ ) {
    assert( p_->has_shortcut() );
    auto it = _shortcuts.find( p_->shortcut() );
    if( it != _shortcuts.end() ) {
        emraise( nonUniq, "Configuration \"%s\":%p already has shortened option '-%c'.",
                this->name(), this,
                p_->shortcut() );
    }
    _shortcuts.emplace( p_->shortcut(), p_ );
}

void
Configuration::_cache_parameter_by_full_name( const std::string & fullName,
                                              iSingularParameter * p_ ) {
    assert( !p_->has_shortcut() );
    assert( p_->name() );
    auto it = _longOptions.find( fullName );
    if( it != _longOptions.end() ) {
        emraise( nonUniq, "Configuration \"%s\":%p already has option '--%s'.",
                this->name(), this,
                fullName.c_str() );
    }
    _longOptions.emplace( fullName.c_str(), p_ );
}

const iSingularParameter &
Configuration::get_parameter( const char path[] ) const {
    assert( path );
    char * path2extract = strdup( path );
    const iSingularParameter & result = _get_parameter( path2extract );
    free( path2extract );
    return result;
}

const iSingularParameter &
Configuration::operator[]( const char path[] ) const {
    return get_parameter(path);
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
}  // namespace dict

# endif

