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

# include "goo_dict/dict.hpp"
# include "goo_dict/parameter.tcc"
# include "goo_dict/insertion_proxy.tcc"

# include <getopt.h>
# include <cstring>

# if 1

namespace goo {
namespace dict {

Dictionary::Dictionary( const Dictionary & orig ) {
    for( auto it  = orig.DictionaryIndex<ListIndex, iBaseValue>::begin();
              it != orig.DictionaryIndex<ListIndex, iBaseValue>::end(); ++it ) {
        if( *it ) {
            insert_item( clone_as<AbstractParameter, AbstractParameter>( *it ) );
        }
    }
}

Dictionary::~Dictionary() {
    for( auto it = Parent::begin();
             it != Parent::end(); ++it ) {
        if( *it ) {
            delete *it;
        }
    }
}

/** Dictionary class design emplies that lifetime of parameters inserted with
 * this method is controlled by dictionary instance. User routines must take
 * this into account: parameter instance inserted by ptr with this method will
 * be deleted by dictionary destructor. */
void
Dictionary::insert_parameter( iSingularParameter * instPtr ) {
    bool wasIndexed = false;
    if( instPtr->has_shortcut() ) {
        auto insertionResult = _parametersIndexByShortcut
                                    .emplace( instPtr->shortcut(), instPtr );
        if( !insertionResult.second ) {
            emraise( nonUniq, "Duplicated option shortcut insertion: "
                "'%c' character was previously associated within parameter %p "
                "(%s). Unable to index parameter %p (%s).", instPtr->shortcut(),
                insertionResult.first->second,
                (insertionResult.first->second->name() ? insertionResult.first->second->name() :
                                                            "<no-long-name>"),
                instPtr,
                (instPtr->name() ? instPtr->name() : "<no-long-name>") );
        }
        wasIndexed = true;
    }
    if( instPtr->name() ) {
        auto insertionResult = _parametersIndexByName
                                        .emplace( instPtr->name(), instPtr );
        if( !insertionResult.second ) {
            emraise( nonUniq, "Duplicated option name insertion: "
                "'%s' name was previously associated within parameter %p. "
                "Unable to index parameter %p.", instPtr->name(),
                insertionResult.first->second, instPtr );
        }
        wasIndexed = true;
    }
    if(!wasIndexed) {
        emraise( badArchitect, "Got %p parameter without name and shortcut.",
                                                                    instPtr );
    }
    SingularsContainer::push_back( instPtr );
}

/** Dictionary class design emplies that lifetime of sub-dictionaries inserted
 * with this method is controlled by this (their parent) dictionary instance.
 * User routines must take this into account: dictionaries (sections)
 * instances inserted by ptr with this method will be deleted by owning
 * dictionary destructor. */
void
Dictionary::insert_section( DictionaryParameter * instPtr ) {
    auto insertionResult = DictionariesContainer::emplace( instPtr->name(), instPtr );
    if( !insertionResult.second ) {
        emraise( nonUniq, "Duplicated subsection name on insertion: "
                "'%s' name was previously associated within subsection %p. "
                "Unable to index new subsection %p with same name.",
                instPtr->name(), insertionResult.first->second, instPtr );
    }
}

# if 0
void
Dictionary::_append_configuration_caches(
                const std::string & nameprefix,
                Configuration * conf
            ) const {
    // Note, that nameprefix already contains this section name.
    for( auto it  = _parametersIndexByShortcut.cbegin();
              it != _parametersIndexByShortcut.cend(); ++it ) {
        conf->_cache_parameter_by_shortcut( it->second );
    }
    for( auto it  = _parametersIndexByName.cbegin();
              it != _parametersIndexByName.cend(); ++it ) {
        conf->_cache_parameter_by_full_name( nameprefix + "." + it->first,
                                                            it->second );
    }
    for( auto it  = _dictionaries.cbegin();
              it != _dictionaries.cend(); ++it ) {
        it->second->_append_configuration_caches(
                nameprefix + "." + it->first, conf
            );
    }
}
# endif

int
Dictionary::pull_opt_path_token( char *& path
                               , char *& current
                               , long & idx ) {
    // if first char is digit, we'll interpret token as integer index:
    int rc = 0;
    if( '#' == *path ) {
        rc |= 0x1;
        ++path;
    }
    current = path;
    for( ; *path != '\0'; ++path ) {
        if( '.' == *path ) {
            *path =  '\0';
            ++path;
            rc |= 0x2;
            break;
        }
        if( !isalnum(*path)
            && '-' != *path
            && '_' != *path ) {
            fprintf( stderr, "Path specification invalid: contains "
                    "character %#x which is not allowed.\n", *path );
            abort();
        }
    }
    if( 0x1 & rc ) {
        // Parse index and check its validity
        char * endPtr;
        idx = strtol( current, &endPtr, 0 );
        if( endPtr != ('\0' != *path ? path-1 : path) ) {
            fprintf( stderr, "Path specification invalid: bad"
                            " token: \"%s\" while parsing digits.\n",
                    current );
            abort();
        }
        if( errno ) {
            fprintf( stderr, "Path specification invalid: strol()"
                    "has set errno=%d: %s\n", errno, strerror(errno) );
            abort();
        }
    }
    return rc;
}

const iSingularParameter *
Dictionary::_get_parameter( char path[], bool noThrow ) const {
    // TODO: recursive part must be a dedicated method.
    char * current;
    long index = 0;
    int rc = pull_opt_path_token( path, current, index );
    if(!(0x2 & rc)) {
        // terminal case --- consider the `current' indexes
        // an option and acquire the value.
        if( 0x1 & rc ) {
            // Latest extracted token is a integer digit that must be
            // interpreted as a list index. Within dictionary, this case has to
            // be considered as an error.
            emraise( badParameter, "Path specification endpoint refers to a"
                    " list (considering integer index %ld) while retrieved"
                    " object is a dictionary.", index );
        } if( path - current > 1 ) {
            // option parameter indexed by long name
            auto it = _parametersIndexByName.find( current );
            if( it != _parametersIndexByName.end() ) {
                return it->second;
            }
            if( !noThrow ) {
                emraise( notFound,
                     "Option \"%s\" (long name considered) not found in "
                     "%p",
                     current, this);
            } else {
                return nullptr;
            }
        } else if( path - current == 0 ) {
            emraise( badState, "Unexpected state of option path parser --- "
                    " null path/name specification length." );
        } else {
            // option parameter indexed by shortcut
            auto it = _parametersIndexByShortcut.find( *current );
            if( it == _parametersIndexByShortcut.end() ) {
                if( !noThrow ) {
                    emraise( notFound,
                         "Option \"%s\" (shortcut considered) not found in "
                         "section %p",
                         current, this );
                } else {
                    return nullptr;
                }
            }
            return it->second;
        }
    } else {
        // Proceed recursively within section -- `current' contains
        // section name (or index refers to a n-th list parameter) and the
        // `path' leads to an option or element.
        if( 0x1 & rc ) {
            // Extracted token is
            // ...
            _TODO_
        } else {
            auto it = DictionariesContainer::find( current );
            if( it == DictionariesContainer::end()) {
                if( !noThrow ) {
                    emraise( notFound, "Subsection \"%s\" is not found in section %p", current, this );
                } else {
                    return nullptr;
                }
            }
            return it->second->_get_parameter( path, noThrow );
        }
    }
}

const iSingularParameter &
Dictionary::parameter( const char path [] ) const {
    return *_get_parameter( strdupa( path ) );
}

const iSingularParameter *
Dictionary::probe_parameter( const char path [] ) const {
    return _get_parameter( strdupa( path ), true );
}

const iSingularParameter *
Dictionary::probe_parameter( const std::string & sPath ) const {
    return _get_parameter( strdupa( sPath.c_str() ), true );
}

const DictionaryParameter *
Dictionary::_get_subsection( char path[], bool noThrow ) const {
    char * current;
    long index;
    int rc = pull_opt_path_token( path, current, index );
    if( 0x1 & rc ) {
        if( noThrow ) {
            return nullptr;
        }
        emraise( badParameter, "Unable to retrieve entry of dictionary by"
                " integer index path specification \"%s\" -> %ld."
               , current, index );
    }
    auto it = DictionariesContainer::find( current );
    if( DictionariesContainer::end() != it ) {
        if( 0 == rc ) {
            // terminal case --- consider the `current' refers to one of
            // own sections:
            return it->second;
        } else {
            // proceed recursively within section -- `current' contains
            // section name and the `path' leads to an option.
            return it->second->_get_subsection( path, noThrow );
        }
    }
    // TODO: code below duplicates parameter retrieval procedure and shall be
    // deleted.
    const iSingularParameter * listParPtr = nullptr;
    if( '\0' == current[1] ) {
        auto spIt = _parametersIndexByShortcut.find( current[0] );
        if( _parametersIndexByShortcut.end() != spIt ) {
            listParPtr = spIt->second;
        }
    } else {
        auto npIt = _parametersIndexByName.find( current );
        if( _parametersIndexByName.end() != npIt ) {
            listParPtr = npIt->second;
        }
    }
    if( !listParPtr ) {
        if( noThrow ) {
            return nullptr;
        }
        emraise( notFound, "Dictionary %p has no section named \"%s\"."
               , this, current );
    }
    auto lst = dynamic_cast<const iParameter<List<
                iStringConvertibleParameter*>> *>( listParPtr );
    if( !lst ) {
        if( noThrow ) {
            return nullptr;
        }
        emraise( notFound, "Dictionary %p has no section named \"%s\" (there is"
                " parameter of the same name)."
               , this, current );
    }
    _TODO_  // TODO: retireve from list
    // ...
}

const DictionaryParameter &
Dictionary::subsection( const char path[] ) const {
    return *_get_subsection( strdupa(path) );
}

const DictionaryParameter *
Dictionary::probe_subsection( const char path[] ) const {
    return _get_subsection( strdupa(path), true );
}

bool
Dictionary::is_consistant( std::map<std::string, const iSingularParameter *> & badParameters,
                           const std::string & prefix ) const {
    for( auto it  = SingularsContainer::cbegin();
              it != SingularsContainer::cend(); ++it ) {
        if( (*it)->is_mandatory() && !(*it)->is_set() ) {
            if( (*it)->name() ) {
                badParameters.emplace(
                        prefix + (*it)->name(),
                        *it
                    );
            } else {
                char bf[2] = {(*it)->shortcut(), '\0'};
                badParameters.emplace(
                    bf,
                    *it
                );
            }
        }
    }
    for( auto it  = DictionariesContainer::cbegin();
              it != DictionariesContainer::cend(); ++it ) {
        it->second->is_consistant( badParameters, prefix + it->first );
    }
    return badParameters.empty();
}

void
Dictionary::print_ASCII_tree( std::list<std::string> & output ) const {
    std::stringstream ss;
    size_t n = SingularsContainer::size();
    bool hasSubsections = !SingularsContainer::empty();
    for( auto p : *static_cast<const SingularsContainer *>(this) ) {
        n--;
        ss << (n || hasSubsections ? "╟─" : "╙─") << " ";
        if( p->name() ) {
            ss << ESC_CLRUNDRLN << p->name() << ESC_CLRCLEAR;
            if( p->has_shortcut() ) {
                ss << "|";
            }
        }
        if( p->has_shortcut() ) {
            ss << ESC_CLRBOLD << p->shortcut() << ESC_CLRCLEAR;
        }
        ss << ", type=`" << p->target_type_info().name() << "'";
        if( p->is_flag() ) {
            ss << ", flag";
        }
        if( p->is_mandatory() ) {
            ss << ", required";
        }
        if( p->has_multiple_values() ) {
            ss << ", array";
        }
        if( p->is_set() ) {
            ss << ", value set: \"" << p->to_string() << "\"";
        }
        // TODO: somehow reflect other parameter properties?
        if( p->description() ) {
            ss << " " << p->description();
        }
        ss << std::endl;
    }
    n = DictionariesContainer::size();
    for( auto dctPair : *static_cast<const DictionariesContainer *>(this) ) {
        n--;
        std::list<std::string> sub;
        dctPair.second->print_ASCII_tree( sub );
        ss << (n ? "╠═" : "╚═")
            << (sub.empty() ? "═" : "╦" )
            << " < " ESC_CLRGREEN << dctPair.first
            << ESC_CLRCLEAR " > " << std::endl;
        for( auto line : sub ) {
            ss << (n ? "║ " : "  ") << line << std::endl;
        }
    }
    std::string line;
    while( std::getline(ss, line) ) {
        output.push_back( line );
    }
}

void
Dictionary::_mark_last_inserted_as_required() {
    if( SingularsContainer::empty() ) {
        emraise( badState,
            "None parameters were set to dictionary, but marking last as "
            "required was requested." );
    }
    SingularsContainer::back()->set_is_argument_required_flag();
}

// ---

DictInsertionProxy
DictionaryParameter::insertion_proxy() {
    return DictInsertionProxy( this );
}

DictionaryParameter::DictionaryParameter( const char * name_,
                        const char * description_ ) :
            DuplicableParent(name_,
                             description_,
                             0 ) {
}

DictionaryParameter::DictionaryParameter( const DictionaryParameter & o ) :
        DuplicableParent(o), Dictionary(o) {}

}  // namespace dict
}  // namespace dict

# endif

