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

# include "goo_dict/appCfg/configuration.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"
# include "goo_exception.hpp"
# include "goo_utility.hpp"
# include "goo_dict/util/conf_help_render.hpp"

# include <algorithm>
# include <cstring>
# include <unistd.h>
# include <getopt.h>
# include <wordexp.h>
# include <list>
# include <cassert>
# include <sstream>
# include <climits>
# include <iostream>

namespace goo {
namespace dict {

// TODO: use own _alloc instead of new for allocating description aspect.
Configuration::Configuration( const std::string & descr_) \
        : DuplicableParent( std::make_tuple(new aspects::Description(descr_) ) )
        , _shortcutsIndex( std::make_tuple<>() )
        , _positionalArgument("", nullptr) {}

Configuration::Configuration( const Configuration & orig ) \
        : DuplicableParent( orig )
        , _shortcutsIndex( std::make_tuple<>() )
        , _positionalArgument("", nullptr) {
    // Upon initial construction procedures are done within the dictionary and
    // aspect copy ctrs, the all the parameters have to be correctly copied
    // except the ones related to configuration itself, i.e. the index of
    // shortcuts, positional and forwarded arguments.
    auto ip = _shortcutsIndex.insertion_proxy();
    //goo::utils::ConfDictCache cc( orig );  // non-const?
    _TODO_
}

Configuration::~Configuration() {
    _shortcutsIndex.drop_entries();
}

void
Configuration::_add_shortcut( char c, FeaturedBase * p ) {
    auto ir = _shortcutsIndex.insertion_proxy().insert(c, p);
    if( !ir.second ) {
        emraise( nonUniq, "Unable to insert parameter %p by shortcut '%c'"
            " since character is already taken by parameter %p.", p, c, ir.first->second );
    }
}

}  // namespace dict

namespace utils {

struct RecursiveVisitor : public dict::AppConfTraits
                                    ::IndexBy<std::string>
                                    ::Aspect
                                    ::RecursiveRevisingVisitor<getopt_ConfDictCache&> {
    typedef dict::AppConfTraits
                ::IndexBy<std::string>
                ::Aspect
                ::RecursiveRevisingVisitor<getopt_ConfDictCache&> Parent;

    std::string & _nameprefix;
    std::stack<std::string> _nameStack;

    /// Overrides subsection callble
    virtual void operator()( SubsectionIterator it ) override {
        _nameStack.push( it->first );
        _TODO_  //_nameprefix = join();  // See: https://stackoverflow.com/a/5289170/1734499
        Parent::operator()( it );
        _nameStack.pop();
    }

    RecursiveVisitor( getopt_ConfDictCache & c
                   , std::string & nameprefixRef ) : Parent( c )
                                                  , _nameprefix(nameprefixRef) {}
};

const char getopt_ConfDictCache::defaultPrefix[8] = "-:";  // "-:h::"

# if 0
void
IConfDictCache::cache_long_options( const std::string & nameprefix
                                , const dict::AppConfNameIndex & D
                                , IConfDictCache & self ) {
    // Fill from this section
    for( const auto & pE : D.value() ) {
        // ...
    }
    // Recursive fill from subsections
    for( auto it = D.subsections().begin()
       ; D.subsections().end() != it; ++it ) {
        cache_long_options( nameprefix + "." + it->first
                           , *(it->second)
                           , self );
    }
}
# endif

void
getopt_ConfDictCache::consider_entry( const std::string & name
                                   , const std::string & nameprefix
                                   , dict::AppConfTraits::FeaturedBase & db ) {
    # if 0
    // ...
    auto * ar = pE.second->aspect_cast<dict::aspects::Required>();
    assert(ar);
    int hasArg = no_argument;
    if( ar->requires_argument() ) {
        hasArg = required_argument;
    }
    if( ar->may_be_set_implicitly() ) {
        //assert( ar->requires_argument() );
        hasArg = optional_argument;
    }
    auto * acs = pE.second->aspect_cast<dict::aspects::CharShortcut>();
    int cshrt = acs->shortcut();
    if( ! cshrt ) {
        // Parameter entry has no shortcut and, thus, has to be accessed via
        // the fully-qualified name only.
        cshrt = UCHAR_MAX + (int) self._lRefs.size();
    }
    struct ::option o = { strdup( (nameprefix + pE.first).c_str() )
                       , hasArg
                       , cshrt >= UCHAR_MAX ? &(self._longOptKey) : NULL
                       , cshrt };
    self._longs.push_back( o );
    if( cshrt >= UCHAR_MAX ) {
        self._lRefs.emplace( cshrt, pE.second );
    }
    # endif
}

getopt_ConfDictCache::getopt_ConfDictCache( dict::Configuration & cfg
                                         , bool dftHelpIFace ) : _dftHelpIFace(dftHelpIFace)
                                                               , _posArgPtr(cfg.positional_argument_ptr())
                                                               , _nameStack(nullptr) {
    // Fill short options string ("optstring" arg for getopt()
    _shorts = defaultPrefix;
    for( auto & sRef : cfg.short_opts().value() ) {
        char c = sRef.first;
        assert( 'W' != c );
        _shorts.push_back(c);
        auto ra = sRef.second->aspect_cast<dict::aspects::Required>();
        assert( !! ra );  // must be excluded upon insertion into shortcuts dict in Configuration
        if( ra->requires_argument() ) {
            _shorts.push_back( ':' );
        }
        if( ra->requires_argument() && ra->may_be_set_implicitly() ) {
            # if 0  // valid case
            if( ! ra->requires_argument() ) {
                emraise( badState, "Option '%c' has contradictory flags:"
                    " it may be implicitly set, but does not expect an argument"
                    " value.", c );
            }
            # endif
            // Two colons mean an option takes an option argument according to
            // getopt's docs.
            _shorts.push_back(':');
        }
    }
    if( dftHelpIFace ) {
        _shorts += "h::";
    }
    // Form long options cache recursively.
    RecursiveVisitor rv(*this);
    _nameStack = &(rv._nameStack);
    cfg.each_subsection_revise( rv );
    _nameStack = nullptr;
    if( dftHelpIFace ) {
        struct ::option helpO = { "help", optional_argument, NULL, 'h' };
        _longs.push_back( helpO );
    }
    struct ::option sentinelO = { NULL, 0, NULL, 0 };
    _longs.push_back( sentinelO );
}

dict::Configuration::FeaturedBase *
getopt_ConfDictCache::current_long_parameter( int optIndex ) {
    if( -1 == optIndex ) {
        emraise( badState, "`getopt_long()` parser has came to the wrong"
               " state: long option index is not being set.");
    }
    assert(optIndex < (int) _longs.size());
    // ?
    assert( _longOptKey >= UCHAR_MAX );
    auto it = _lRefs.find( _longOptKey );
    assert( _lRefs.end() != it );
    return it->second;
}

int
set_app_conf( dict::Configuration & cfg
            , int argc
            , char * const * argv
            , getopt_ConfDictCache * cachePtr
            , std::ostream *logStreamPtr ) {
    bool ownCacheStruct = ! cachePtr;
    # define log_extraction( ... ) if( !!logStreamPtr ) { *logStreamPtr << strfmt( __VA_ARGS__ ); }
    ::opterr = 0;  // prevent default `app_name : invalid option -- '%c'' message
    ::optind = 0;  // forces rescan with each parameters vector
    if( ownCacheStruct ) {
        cachePtr = new getopt_ConfDictCache( cfg );
    }
    getopt_ConfDictCache & C = *cachePtr;
    if( !! logStreamPtr ) {
        log_extraction( "parserCaches:\n    shortOptions: \"%s\"\n"
                                "    longOptions:\n",
                C.shorts().c_str() );
        size_t n = 0;
        for( const struct ::option & loE : C.longs() ) {
            if( !loE.name ) {
                *logStreamPtr << std::string(8, ' ') << "<sentinel>" << std::endl;
                continue;
            }
            *logStreamPtr << std::string(8, ' ') << loE.name << ":" << std::endl;
            *logStreamPtr << std::string(12, ' ') << "argument: "
                     << (required_argument == loE.has_arg ? "required" :
                        (optional_argument == loE.has_arg ? "optional" : "none") )
                     << std::endl
                     << std::string(12, ' ') << "getoptVal: "
                     << std::hex << std::showbase << loE.val
                     << std::endl;
            ++n;
        }
    }
    // TODO: check whether getopt_long arguments (composed caches) are empty.
    // If they are --- shall we raise an exception? How about shortened-only
    // case or even when only positional arguments are provided?
    int optIndex = -1, c;

    while( -1 != (c = getopt_long( argc, argv
                                 , cachePtr->shorts().c_str()
                                 , cachePtr->longs().data()
                                 , &optIndex )) ) {
        // This pointer has to be set within if-clauses below
        dict::Configuration::FeaturedBase * pPtr = nullptr;
        //if ( optind == prevInd + 2 && *optarg == '-' ) {
        //    c = ':';
        //    -- optind;
        //}
        if( isalnum(c) ) {
            if( 'h' == c && cachePtr->default_help_interface() ) {
                if( !optarg || !strnlen(optarg, USHRT_MAX) ) {
                    log_extraction( "Got help reference option without argument,"
                        " abort extraction now." );
                    // No argument given --- print default usage text.
                    std::cout << compose_reference_text_for( cfg );
                    return -1;
                } else {
                    log_extraction( "Got help reference option with argument \"%s\","
                        " abort extraction now.", optarg );
                    // Section name given --- print subsection reference.
                    std::cout << compose_reference_text_for(cfg, optarg );
                    return -1;
                }
            }
            // indicates this is an option with shortcut (or a shortcut-only option)
            auto pIt = cfg.short_opts().value().find( c );
            if( cfg.short_opts().value().end() == pIt ) {
                emraise( badState, "Shortcut option '%c' (0x%02x) unknown."
                       , c, c );
            }
            pPtr = pIt->second;
            log_extraction( "Recognized short option '%c' as parameter %p.\n"
                         , c, pPtr );
        } else if( 0 == c ) {
            pPtr = cachePtr->current_long_parameter( optIndex );
            log_extraction( "Recognized long parameter %p ( optindex=%d, \"%s\")...\n"
                         , pPtr
                         , (int) optIndex
                         , cachePtr->longs().data()[optIndex].name
                         );
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
            emraise( argumentExpected, "Parameter option expects an argument." );
        } else if(  01 == c ) {
            // this is a positional argument (because we have set the '-'
            // character first in optstring).
            log_extraction( "\"%s\" recognized as a positional argument.\n"
                         , ::optarg );
            pPtr = cachePtr->positional_arg_ptr();
        } else {
            emraise( badValue, "getopt_long() returned code %#02x.", c );
        }
        assert(pPtr);
        // Upon now, the look-up procedure has to came up with the particular
        // parameter entry instance that may be set (or appended) with the
        // argument value (or may be not, depending on its internal properties
        auto ar = pPtr->aspect_cast<dict::aspects::Required>();
        assert( !!ar );
        if( ar->requires_argument() ) {
            //log_extraction( "c=%c (0x%02x) considered as a (short)"
            //               " parameter with argument \"%s\".\n"
            //             , c, c, optarg );
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
            set_app_cfg_parameter( *pPtr, optarg, logStreamPtr );
        } else {
            ar->assign_implicit_value();
            log_extraction( "    parameter %p has been implicitly set.\n", pPtr );
        }

        optIndex = -1;
    }
    // appending of positional arguments, if any:
    if( ::optind < argc ) {
        while( optind < argc ) {
            set_app_cfg_parameter( *(cachePtr->positional_arg_ptr())
                                , argv[optind++]
                                , logStreamPtr );
            log_extraction( "\"%s\" recognized as a positional argument "
                                    "(on argv-tail processing).\n",
                    argv[optind-1] );
        }
    }
    # undef log_extraction
    if( ownCacheStruct ) {
        delete cachePtr;
    }
    return 0;
    // ...
}

void set_app_cfg_parameter( dict::Configuration::FeaturedBase & v
                          , const char * strExpr
                          , std::ostream * logPtr ) {
    auto scnv = v.aspect_cast<dict::aspects::iStringConvertible>();
    assert( scnv );
    scnv->parse_argument( strExpr );
    if( logPtr ) {
        *logPtr << "    " << (void *) &v << " set to "
                << scnv->to_string() << std::endl;
    }
}

std::string
compose_reference_text_for( const dict::Configuration & cfg
                         , const std::string & sect ) {
    _TODO_  // TODO
}

}  // namespace utils
}  // namespace goo

