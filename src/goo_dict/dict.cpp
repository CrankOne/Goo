# include "goo_dict/dict.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_exception.hpp"

# include <cstring>
# include <unistd.h>
# include <getopt.h>

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
    for( auto it = _dictionaries.begin();
         it != _dictionaries.end(); ++it ) {
        delete it->second;
    }
}

void
Dictionary::insert_parameter( iAbstractParameter * instPtr ) {
    _parameters.emplace( instPtr->name(),
                         instPtr );
}

void
Dictionary::insert_section( Dictionary * instPtr ) {
    _dictionaries.emplace( instPtr->name(),
                           instPtr );
}

// iAbstractParameter interface implementation
//////////////////////////////////////////////

const void *
Configuration::_form_long_options() const {
    # if 0
    // ...
    return _longOptionsPtr;
    # else
    struct ::option * lOpts = new struct ::option [7];
    lOpts[0] = {"add",     required_argument, 0,  0 };
    lOpts[0] = {"append",  no_argument,       0,  0 };
    lOpts[0] = {"delete",  required_argument, 0,  0 };
    lOpts[0] = {"verbose", no_argument,       0,  0 };
    lOpts[0] = {"create",  required_argument, 0, 'c'};
    lOpts[0] = {"file",    required_argument, 0,  0 };
    lOpts[0] = {0,         0,                 0,  0 };
    return _longOptionsPtr = lOpts;
    # endif
}

const char *
Configuration::_create_short_opt_string() const {
    # if 0
    // ...
    return _shortOptionsPtr;
    # else
    const char src[] = "-:abc:d:012";
    char * sOpts = new char [ strlen(src) + 1 ];
    strcpy( sOpts, src );
    return _shortOptionsPtr = sOpts;
    # endif
}

# if 0
void
Configuration::command_line_argument_to_tokens( int argc,
                                                char * argv[],
                                                struct Configuration::Tokens & tokens ) {
    if( argc < 2 ) {
        tokens.positionalValues.clear();
        tokens.options.clear();
        return;
    }
    // argv[0]  --- omit name of the application.
    for( int i = 1; i < argc; ++i ) {
        const char * const & argument = argv[i];
        const size_t argLength = strlen( argument );
        if( !argLength ) {
            emraise(malformedArguments, "Got an empty argument at %d-th position.", i);
        }
        if( '-' == argument[0] ) {  // guess, this is an option
            // ...
        } else {  // this is an argument
        }
    }
}
# endif

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
    const struct ::option * longOptions = reinterpret_cast<const ::option *>( _form_long_options() );
    const char * shrtOptStr = _create_short_opt_string();
    int optIndex = 0, c;
    while( -1 != (c = getopt_long( argc, argv, shrtOptStr, longOptions, &optIndex )) ) {
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
                                                      _longOptionsPtr(nullptr),
                                                      _shortOptionsPtr(nullptr) {
}

Configuration::~Configuration() {
    if( _longOptionsPtr ) {
        delete [] reinterpret_cast<::option *>(_longOptionsPtr);
    }
    if( _shortOptionsPtr ) {
        delete [] _shortOptionsPtr;
    }
}

void
Configuration::usage_text( std::ostream & os,
                           bool enableASCIIColoring ) {
    os << name() << " --- " << description() << std::endl
       << "Usage:" << std::endl;
}

}  // namespace dict
}  // namespace dict

# endif

