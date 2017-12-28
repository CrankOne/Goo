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
    const SingularsContainer & singRef = orig;
    const DictionariesContainer & dictsRef = orig;
    for( auto it  = singRef.begin();
              it != singRef.end(); ++it ) {
        if( *it ) {
            insert_parameter( clone_as<iAbstractParameter,
                              iSingularParameter>( *it ) );
        }
    }
    for( auto it  = dictsRef.begin();
              it != dictsRef.end(); ++it ) {
        insert_section( clone_as<iAbstractParameter,
                DictionaryParameter>( it->second ) );
    }
}

Dictionary::~Dictionary() {
    //const SingularsContainer & singRef = ;
    //const DictionariesContainer & dictsRef = orig;
    for( auto it = SingularsContainer::begin();
         it != SingularsContainer::end(); ++it ) {
        if( *it ) {
            delete *it;
        }
    }
    for( auto it = DictionariesContainer::begin();
         it != DictionariesContainer::end(); ++it ) {
        delete it->second;
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
                               , size_t & idx ) {
    current = path;
    for( ; *path != '\0'; ++path ) {
        if( '.' == *path ) {
            *path = '\0';
            ++path;
            break;
        }
        if( '[' == *path ) {
            *(path++) = '\0';
            current = path;
            char * digitStart = path;
            while(isdigit(*(++path))) {}
            if( ']' != *path ) {
                emraise( badParameter, "Path specification invalid: character ']'"
                        " expected to close digits after '['. Got character %0x"
                        " instead.", *path );
            }
            if( path - digitStart > 1 ) {
                *path = '\0';
                idx = atoi(digitStart);
                return 2;
            }
            emraise( badParameter, "Path specification invalid: empty"
                     " index provided." );
        }
        if( !isalnum(*path)
            && '-' != *path
            && '_' != *path ) {
            emraise( badParameter, "Path specification invalid: contains "
                    "character %0x which is not allowed.", *path );
        }
    }
    if( '\0' == *path ) {
        return 0;  // no more tokens
    }
    return 1;  // tail is not empty
}

const iSingularParameter *
Dictionary::_get_parameter( char path[], bool noThrow ) const {
    char * current;
    int rc = pull_opt_path_token( path, current );
    if( rc < 0 ) {
        // terminal case --- consider the `current' indexes
        // an option and acquire the value.
        if( path - current > 1 ) {
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
                               "null option length."
                               "See sources for details." );
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
        // proceed recursively within section -- `current' contains
        // section name and the `path' leads to an option.
        auto it = DictionariesContainer::find( current );
        if( it == DictionariesContainer::end() ) {
            if( !noThrow ) {
                emraise( notFound,
                         "Subsection \"%s\" is not found in section %p",
                         current, this );
            } else {
                return nullptr;
            }
        }
        return it->second->_get_parameter( path, noThrow );
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
    int rc = pull_opt_path_token( path, current );
    auto it = DictionariesContainer::find( current );
    if( DictionariesContainer::end() == it ) {
        if( !noThrow ) {
            emraise( notFound, "Dictionary %p has no section named \"%s\".",
                    this, current );
        } else {
            return nullptr;
        }
    }
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

