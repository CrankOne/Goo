# include "goo_dict/dict.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_exception.hpp"

# include <cstring>
# include <unistd.h>
# include <getopt.h>
# include <wordexp.h>
# include <list>

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
    std::list<::option *> lOpts;
    std::string sOpts = "-:";

    //for( auto it = _byShortcutIndexed.cbegin();
    //          _byShortcutIndexed.end() != it; ++it ) {
    //    //
    //}

    _TODO_ // TODO: copy lOpts / sOpts to _cache_*s

    _getoptCachesValid = true;
}

void
Configuration::extract( int argc, char * const argv[] ) {
    # if 0
    int c;
    while (1) {
        static struct option long_options[] = {
                /* These options set a flag. */
                {"verbose", no_argument,       &verbose_flag, 1},
                {"brief",   no_argument,       &verbose_flag, 0},
                /* These options don’t set a flag.
                   We distinguish them by their indices. */
                {"add",     no_argument,       0, 'a'},
                {"append",  no_argument,       0, 'b'},
                {"delete",  required_argument, 0, 'd'},
                {"create",  required_argument, 0, 'c'},
                {"file",    required_argument, 0, 'f'},
                {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "abc:d:f:",
                        long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
            break;

            case 'a':
                puts ("option -a\n");
            break;

            case 'b':
                puts ("option -b\n");
            break;

            case 'c':
                printf ("option -c with value `%s'\n", optarg);
            break;

            case 'd':
                printf ("option -d with value `%s'\n", optarg);
            break;

            case 'f':
                printf ("option -f with value `%s'\n", optarg);
            break;

            case '?':
                /* getopt_long already printed an error message. */
            break;

            default:
                abort ();
        }
    }
    /* Instead of reporting ‘--verbose’
       and ‘--brief’ as they are encountered,
       we report the final status resulting from them. */
    if (verbose_flag)
        puts ("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        putchar ('\n');
    }
    exit (0);
    # endif
    opterr = 0;  // prevent default `app_name : invalid option -- '%c'' message
    optind = 0;  // forses rescan with each parameters vector
    if( !_getoptCachesValid ) {
        _recache_getopt_arguments();
    }
    const struct ::option * longOptions = reinterpret_cast<const ::option *>( _cache_longOptionsPtr );
    int optIndex = 0, c;
    while( -1 != (c = getopt_long( argc, argv, _cache_shortOptionsPtr, longOptions, &optIndex )) ) {
        if( isalnum(c) ) {
            // TODO: indicates this is an option with shortcut (or a shortcut-only option)
        } else if( 1 == c ) {
            // TODO: indicates this is a kind of long option (without arg)
        } else if( 2 == c ) {
            // TODO: indicates this is a long parameter (with arg)
        } else if( '?' == c ) {
            // TODO: fprintf( stderr, "Unrecognized option `%c' (%d).\n", optopt, (int) optopt );
        } else if( ':' == c ) {
            // TODO: fprintf( stderr, "Missed option argument.\n" );
        } else if(  01 == c ) {
            // TODO: this is a positional argument
        } else {
            // TODO: fprintf( stderr, "?? getopt returned character code 0%o ??\n", c);
        }
    }
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

Configuration::~Configuration() {
    if( _cache_longOptionsPtr ) {
        delete [] reinterpret_cast<::option *>(_cache_longOptionsPtr);
    }
    if( _cache_shortOptionsPtr ) {
        delete [] _cache_shortOptionsPtr;
    }
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
}

void
Configuration::insert_section( Dictionary * sect ) {
    _getoptCachesValid = false;
    Dictionary::insert_section( sect );
}

}  // namespace dict
}  // namespace dict

# endif

