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
# include "goo_sys/exception.hpp"
# include "goo_utility.hpp"
# include "goo_dict/util/conf_help_render.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) \
  && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

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
namespace utils {

//
// getopt() internal cache

/// Aux class collecting the getopt() options.
struct getopt_ConfDictCache {
public:
    /// This two arrays keep the common prefixes for getopt() shortcuts string.
    static const char defaultPrefix[8];

    /// Temporary data attribute, active only during the recursive traversal.
    //mutable NamePrefixStack nameprefix;
    mutable class NamePrefixStack : public std::vector<std::string> {
    private:
        size_t _fullLength;
    public:
        NamePrefixStack() : _fullLength(0) {}
        void push( dict::AppConfTraits::IndexBy<dict::String>::Aspect::Parent::iterator it ) {
            _fullLength += 1 + it->first.size();
            std::vector<std::string>::push_back( it->first.c_str() );
        }
        void pop() {
            _fullLength -= 1 + std::vector<std::string>::back().size();
            assert( _fullLength >= 0 );
            std::vector<std::string>::pop_back();
        }
        size_t full_length() const { return _fullLength; }
        void copy_full_prefix_to( char *, size_t n );
    } nameprefix;
private:
    bool _dftHelpIFace;
    std::string _shorts;
    std::vector<struct ::option> _longs;
    std::unordered_map<int, dict::Configuration::FeaturedBase *> _lRefs;
    dict::Configuration::FeaturedBase * _posArgPtr;
    /// This variable where option identifiers will be loaded into.
    int _longOptKey;
public:
    explicit getopt_ConfDictCache( dict::Configuration & cfg
                                 , bool dftHelpIFace=true );

    virtual ~getopt_ConfDictCache();

    virtual void consider_entry( const std::string & name
                               , dict::AppConfTraits::FeaturedBase & );

    const std::string & shorts() const { return _shorts; }
    const std::vector<struct ::option> & longs() const { return _longs; };
    bool default_help_interface() const { return _dftHelpIFace; }
    dict::Configuration::FeaturedBase * positional_arg_ptr() const { return _posArgPtr; }
    dict::Configuration::FeaturedBase * current_long_parameter( int );

    /// Used to fill caches during recursive traversal:
    void operator()( dict::Hash<std::string, dict::AppConfTraits::FeaturedBase *>::iterator it ) {
        consider_entry( it->first, *(it->second) );
    }
};  // struct getopt_ConfDictCache

typedef goo::dict::aux::RevisingVisitor< getopt_ConfDictCache
                                      , getopt_ConfDictCache::NamePrefixStack &
                                      , dict::AppConfTraits > RecursiveVisitor_getopt;


//
// Copying procedure internal cache

struct copier_ConfDictCache {
    /// Insertion proxy to the
    dict::InsertionProxy<char> ip;

    /// Used to fill caches during recursive traversal:
    void operator()( dict::Hash<std::string, dict::AppConfTraits::FeaturedBase *>::iterator it ) {
        auto acs = it->second->aspect_cast<dict::aspects::CharShortcut>();
        int cshrt = acs->shortcut();
        if( cshrt ) {
            ip.insert( cshrt, it->second );
        }
    }
};

//
// Configuration consistency check

void
AppConfValidator::operator()( dict::Hash< dict::String
                                        , dict::AppConfTraits::FeaturedBase *>::const_iterator it ) {
    uint8_t reason = 0x0;
    auto poa = it->second->aspect_cast<dict::aspects::ProgramOption>();
    auto isa = it->second->aspect_cast<dict::aspects::IsSet>();
    auto dea = it->second->aspect_cast<dict::aspects::Description>();
    //const char * nm = it->first.c_str();  // XXX
    if( (_opts & requiredAreNotSet) && (poa->is_required() && !isa->is_set()) ) {
        reason |= requiredAreNotSet;
    }
    if( (_opts & requiredAreImplicit) && (poa->is_required() && poa->may_be_set_implicitly()) ) {
        reason |= requiredAreImplicit;
    }
    if( (_opts & emptyDescription) && dea->description().empty() ) {
        reason |= emptyDescription;
    }
    // ...
    if( reason ) {
        _badEntries.push_back({ _reentrantStack, it, reason });
    }
}

std::vector<AppConfValidator::InvalidEntry>
AppConfValidator::run(const dict::Configuration & cfg, uint8_t options) {
    AppConfValidator v(options);
    dict::aux::ReadingVisitor< AppConfValidator&
                           , std::stack<dict::AppConfTraits::IndexBy<dict::String>
                                                         ::Aspect::Parent::const_iterator>
                           , dict::AppConfTraits> vv(v);
    cfg.each_subsection_read(vv);
    cfg.each_entry_read<AppConfValidator &>(v);
    return v._badEntries;
}

}  // namespace utils

namespace dict {

// TODO: use own _alloc instead of new for allocating description aspect.
// TODO: track allocator
Configuration::Configuration( const char * descr
                            , DictionaryAllocator<_Goo_m_VART_LIST_APP_CONF> & a ) \
        : DuplicableParent( a, std::make_tuple( _alloc<aspects::Description>(descr, a) ) )
        , _shortcutsIndex( as_allocator(), std::make_tuple<>() )
        , _positionalArgument("", nullptr) {}

// TODO: allocs
Configuration::Configuration( const Configuration & orig
                            , DictionaryAllocator<_Goo_m_VART_LIST_APP_CONF> & a ) \
        : DuplicableParent( orig, a )
        , _shortcutsIndex( a, std::make_tuple<>() )
        , _positionalArgument( "", nullptr ) {
    // Upon initial construction procedures are done within the dictionary and
    // aspect copy ctrs, the all the parameters have to be correctly copied
    // except the ones related to configuration itself, i.e. the index of
    // shortcuts, positional and forwarded arguments.
    auto ip = _shortcutsIndex.insertion_proxy();
    ::goo::utils::copier_ConfDictCache ccdc = { ip };
    dict::AppConfTraits::IndexBy<String>::Aspect
                    ::RecursiveRevisingVisitor<::goo::utils::copier_ConfDictCache&> shrtc(ccdc);
    this->each_subsection_revise(shrtc);
    this->each_entry_revise( ccdc );
    for( auto it = orig._shortcutsIndex.value().begin();
         it != orig._shortcutsIndex.value().end(); ++it ) {
        auto presence = _shortcutsIndex.value().find( it->first );
        if( presence != _shortcutsIndex.value().end() ) {
            continue;  // already copied
        }
        auto pc = goo::clone_as< iAbstractValue
                               , FeaturedBase
                               , FeaturedBase
                               , AbstractValueAllocator & >( it->second, *this );
        ip.insert( it->first, pc );
    }
    if( orig._positionalArgument.second ) {
        _TODO_  // TODO: copy positional args
    }
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

const char getopt_ConfDictCache::defaultPrefix[8] = "-:";  // "-:h::"

void
getopt_ConfDictCache::NamePrefixStack::copy_full_prefix_to( char * s, size_t n ) {
    if( n < _fullLength ) {
        emraise( underflow, "Insufficient string buffer: %zu < %zu.", n, _fullLength );
    }
    if( empty() ) {
        *s = '\0';
        return;
    }
    for( const std::string & token : *this ) {
        strncpy( s, token.c_str(), n );
        n -= token.size() + 1;
        s += token.size();
        *s = '.';  // delimiter
        ++s;
    }
    *s = '\0';
}

void
getopt_ConfDictCache::consider_entry( const std::string & name
                                    , dict::AppConfTraits::FeaturedBase & db ) {
    assert(!name.empty()); //?
    auto * ar = db.aspect_cast<dict::aspects::ProgramOption>();
    assert(ar);
    int hasArg = no_argument;
    if(ar->expects_argument() ) {
        hasArg = required_argument;
    }
    if( ar->may_be_set_implicitly() ) {
        //assert( ar->expects_argument() );
        hasArg = optional_argument;
    }
    auto * acs = db.aspect_cast<dict::aspects::CharShortcut>();
    int cshrt = acs->shortcut();
    if( ! cshrt ) {
        // Parameter entry has no shortcut and, thus, has to be accessed via
        // the fully-qualified name only.
        cshrt = UCHAR_MAX + (int) _lRefs.size();
    }
    size_t n = nameprefix.full_length() + name.size() + 1;
    char * nameStr = (char *) malloc( n );
    nameprefix.copy_full_prefix_to( nameStr, n );
    strcpy( nameStr + nameprefix.full_length(), name.c_str() );
    struct ::option o = { nameStr
                       , hasArg
                       , cshrt >= UCHAR_MAX ? &(_longOptKey) : NULL
                       , cshrt };
    _longs.push_back( o );
    if( cshrt >= UCHAR_MAX ) {
        _lRefs.emplace( cshrt, &db );
    }
}

getopt_ConfDictCache::getopt_ConfDictCache( dict::Configuration & cfg
                                         , bool dftHelpIFace ) : _dftHelpIFace(dftHelpIFace)
                                                               , _posArgPtr(cfg.positional_argument_ptr()) {
    // Fill short options string ("optstring" arg for getopt()
    _shorts = defaultPrefix;
    for( auto & sRef : cfg.short_opts().value() ) {
        char c = sRef.first;
        assert( 'W' != c );
        _shorts.push_back(c);
        auto ra = sRef.second->aspect_cast<dict::aspects::ProgramOption>();
        assert( !! ra );  // must be excluded upon insertion into shortcuts dict in Configuration
        if( ra->expects_argument() ) {
            _shorts.push_back( ':' );
        }
        if( ra->expects_argument() && ra->may_be_set_implicitly() ) {
            # if 0  // valid case
            if( ! ra->expects_argument() ) {
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
    RecursiveVisitor_getopt rv(*this, this->nameprefix);
    cfg.each_subsection_revise( rv );
    cfg.each_entry_revise<getopt_ConfDictCache &>( *this );
    if( dftHelpIFace ) {
        struct ::option helpO = { "help", optional_argument, NULL, 'h' };
        _longs.push_back( helpO );
    }
    struct ::option sentinelO = { NULL, 0, NULL, 0 };
    _longs.push_back( sentinelO );
}

getopt_ConfDictCache::~getopt_ConfDictCache() {}

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
        auto ar = pPtr->aspect_cast<dict::aspects::ProgramOption>();
        assert( !!ar );
        if(ar->expects_argument() ) {
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

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
