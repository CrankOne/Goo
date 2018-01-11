# include <cstring>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

# include "goo_dict/parameters/los.hpp"

namespace goo {
namespace dict {


void
InsertionProxyBase::InsertionTarget::_assert_is( bool requireNamed
                                               , bool requireIsDict
                                               , bool forceRequireNamed ) {
    bool errOccured = false;
    char errbf1[32]
       , errbf2[32]
       ;
    if( requireNamed && !_isNamed ) {
        snprintf( errbf1, sizeof( errbf1 ), "anonymous" );
        errOccured |= true;
    } else if( !requireNamed && _isNamed && forceRequireNamed ) {
        snprintf( errbf1, sizeof( errbf1 ), "named" );
        errOccured |= true;
    }
    if( requireIsDict && !_isDict ) {
        snprintf( errbf1, sizeof( errbf2 ), "list" );
        errOccured |= true;
    } else if( !requireIsDict && _isDict ) {
        snprintf( errbf1, sizeof( errbf2 ), "dictionary" );
        errOccured |= true;
    }
    if( errOccured ) {
        emraise( assertFailed, "Insertion target refers to %s %s object,"
                 " while user code tries to retrieve %s %s (%s). Type "
                 "conflict: target is %s %s."
               , _isNamed ? "named" : "anonymous"
               , _isDict ? "dictionary" : "list"
               , requireNamed ? "named" : "anonymous"
               , requireIsDict ? "dictionary" : "list"
               , forceRequireNamed ? "strictly expecting name" : "allowing anonymous"
               , errbf1
               , errbf2
               );
    }
}

/**This routine serves few use cases, but basically it is designed to perform
 * recursive evaluation according to textual path specified, within given
 * goo::pdict data structure. Few major use-cases are:
 *
 * 1. Non-invasive look-up. Any not found entry will cause goo::notFound
 * exception.
 * 2. Extension. Routine will try to insert new subsections and lists inside
 * the starting entity.
 *
 * Path examples:
 *  - "one.#23" --- for non-invasive look-up this path expression will retrieve
 *  iStringConvertibleParameter pointer to 23-rd element of list "one" in
 *  starting dictionary. In extending mode it will add 23 element to list "one"
 *  in the starting dictionary. If list "one" does not exist or contains less
 *  than 22 elements, even extending look-up mode will emit the exception
 *  (goo::overflow in both cases).
 *  - "#11.#0" --- for non-invasive look-up, will retrieve first element of
 *  element 11 of starting list.
 *  - "one.#" --- possible only in extending mode. Will cause routine to insert
 *  the list named "one" at the starting dictionary.
 *
 *  TODO: test this thing. Very. Carefully.
 *
 *  @param currentStack describes the target stack instance where references to
 *      dictionaries or lists have to be put in.
 *  @param path_ is a textual path.
 *  @param extend controls whether to insert new lists or dictionaries. Being
 *      set to `false` causes routine to emit notFound exception.
 * */
InsertionProxyBase::MaterializedPath
InsertionProxyBase::combine_path( InsertionTargetsStack & mpath
                                , char * path
                                , bool extend
                                , const std::string ed ) {
    // This macro defines finishing block. Depending on whether the last token
    // was extracted from path, the routine will terminate itself, or invoke
    // itself further.
    # define CONTINUE_RECURSIVELY                        \
    if( !lastToken ) {                                   \
        return combine_path( mpath, path, extend, ed );  \
    } else { return mpath; }

    InsertionTarget & start = mpath.top();
    char * current = NULL;
    long index;

    // Pull token from left
    int rc = DictionaryParameter::pull_opt_path_token(path, current, index);

    const bool emptyToken = !strlen(current)
             , lastToken = !! (0x2 & rc)
             , isDict = !! (0x1 & rc)
             ;

    if( emptyToken && (!extend || lastToken) ) {
        emraise( badParameter, "Empty token met in path specification within"
                " non-extending call." );
    }

    if( isDict ) {
        // Token is a string. Consider start as (at least) of type Dict
        // and try to retrieve parameter or subsection named as `current'
        // refers.
        Dict & startDctRef = start.as<Dict>(false);
        {
            NamedDict * newStart = startDctRef.probe_subsection( current );
            if( newStart ) {
                // That's a dict indeed. Continue recursively.
                mpath.push( newStart );
                CONTINUE_RECURSIVELY;
            }
        }
        // Token apparently refers to a parameter
        {
            iSingularParameter * pPtr = startDctRef.probe_parameter( current );
            if( pPtr ) {
                // Parameter found. Try to downcast it to dict and consider
                // next token in a path as an index.
                auto newStart = dynamic_cast<NamedLoS*>( pPtr );
                if( !newStart ) {
                    if( 0x2 & rc ) {
                        // The parameter must be a subclass of LoS in this
                        // context.
                        emraise( notFound, "Interim path token \"%s\" refers to"
                                " existing singular parameter %p within a"
                                " dictionary, but this parameter does not"
                                " support subsequent indexing.", current, pPtr );
                    } else {
                        // Token terminating path refers to a parameter.
                        return MaterializedPath( mpath, pPtr );
                    }
                }
                mpath.push( newStart );
                CONTINUE_RECURSIVELY;
            }
        }
        // Following code corresponds to the case of not found entry within a
        // dictionary
        if( !extend ) {
            std::string nm = start.get_name("<anonymous>");
            emraise( notFound, "No entry \"%s\" found within dictionary"
                   " %p (within %s dict)."
                   , current, &startDctRef, nm.c_str() );
        }
        // To extend a dictionary within given path, we have to assume the
        // current parameter type. It may be either a named dict
        // (subsection), or a list here. The obvious way is to look for the
        // next token in a string path --- whether it starts from # sign.
        if( '#' != *path ) {
            // It is a named dict.
            auto newStart = new NamedDict( current, ed.c_str() );
            startDctRef.insert_section( newStart );
            mpath.push( newStart );
            return combine_path( mpath, path, extend, ed.c_str() );
        } else {
            // It is a named LoS.
            //auto newStart = new NamedLoS( current, ed.c_str() );
            auto newStart = new Parameter<List<iStringConvertibleParameter*>>(current, ed.c_str());
            startDctRef.insert_parameter( newStart );
            mpath.push( static_cast<NamedLoS*>(newStart) );
            return combine_path( mpath, path, extend, ed.c_str() );
        }
        // NOTE: one has to check, whether the situations like "one.two.#" or
        // "one.two." works correctly.
    } else {
        // Token is an integer (index). Consider start as (at least of type
        // LoS and try to retrieve element referenced by this index.
        LoS & startLoSRef = start.as<LoS>(false);
        bool newElement = false;
        if( index >= (long) startLoSRef.value().size() ) {
            if( !extend ) {
                emraise( notFound, "List object %p (%s) has only %zu elements"
                        ", but #%ld-th element is requested."
                         , &startLoSRef
                         , start.get_name( "<anonymous>" ).c_str()
                         , startLoSRef.value().size()
                         , index );
            }
            if( index != startLoSRef.value().size() ) {
                newElement = true;
            } else {
                emraise( overflow, "Unable to extend list %p (%s) currently"
                       " having %zu elements with %ld-th element (number"
                       " has to refer the last element in a list)."
                       , &startLoSRef
                       , start.get_name( "<anonymous>" )
                       , startLoSRef.value().size(),
                       , index );
            }
        }
        // Before we start to handle the element list as a materialized
        // path entry, we have to find out its particular type in terms
        // of goo::dict containers --- whether or not it is a list or a
        // dict. Since we're currently operating within list context both
        // of them have to be anonymous.

        //iStringConvertibleParameter * scpEl;
        /*
        iStringConvertibleParameter * scpEl = startLoSRef.value().at(index);
        // Since path implies, that retrieved element is not the last one,
        // try to cast
        bool isList = '#' == *path;
        if( isList ) {
            LoS * losPtr = dynamic_cast<LoS*>(scpEl);
            if( losPtr ) {
                NamedLoS * nLoSPtr = dynamic_cast<NamedLoS*>(scpEl);
                mpath.push( nLoSPtr );
                return combine_path( mpath, path, extend, ed );
            }
            //mpath
            //return ;
        }
         */
    }

    # if 0
    while(0x2 & (rc = DictionaryParameter::pull_opt_path_token( path
                                                              , current
                                                              , index ))) {
        if( !(rc & 0x1) ) {
            // Extracted token refers to a named parameter or dictionary.
            NamedDict * newStart = start.as<Dict>(false).probe_subsection(current);
            if( !newStart ) {
                // Extracted token refers to non-existing dict.
                if( !extend ) {
                    emraise( notFound, "No subsection named \"%s\" found in"
                            " subsection or dictionary instance %p."
                           , current, &(h._top_as<Dict>(false)) );
                }
                // otherwise, extend
                newTop = new DictionaryParameter( current, ed.c_str() );
                // We invoke the _top_as<> with "Dict" type because current
                // routine may operate with anonymous dict at this level
                // (however, the dict we insert is not anonymous one).
                h._top_as<Dict>(false).insert_section( newTop );
            }
            h._push_dict( newTop );
        } else {
            // Extracted token refers to a parameter with given index.
            _TODO_
        }
    }
    # endif

    _TODO_
    # if 0
    if( !(newTop = _top_as<NamedDict>(true).probe_subsection( current )) ) {
        // Insert new section.
        newTop = new DictionaryParameter( current, descr );
        _top_as<NamedDict>(true).insert_section( newTop );
        _push_dict( newTop );
    } else {
        // Append existing section:
        _push_dict( newTop );
        if( !newTop->description() ) {
            newTop->_append_description( descr );
        }
    }
    return *this;
    # endif
    # undef CONTINUE_RECURSIVELY
}

//
// Dictionary Insertion Proxy
// /////////////////////////

DictInsertionProxy::DictInsertionProxy( DictionaryParameter * root ) :
                    InsertionProxyBase(root) {
    _push_dict( root );
}

void
DictInsertionProxy::insert_copy_of( const iSingularParameter & sp
                                  , const char * newName ) {
    iSingularParameter * isp =
                    clone_as<iAbstractParameter, iSingularParameter>( &sp );
    if( !! newName ) {
        isp->name( newName );
    }
    _top_as<Dict>(false).insert_parameter( isp );
}

DictInsertionProxy &
DictInsertionProxy::required_argument() {
    _top_as<Dict>(false)._mark_last_inserted_as_required();
    return *this;
}

DictInsertionProxy &
DictInsertionProxy::bgn_sect( const char * name, const char * descr) {
    _TODO_
}

DictInsertionProxy &
DictInsertionProxy::end_sect( const char * name ) {
    long index;
    if( name ) {
        char * path = strdupa( name ),
             * current = NULL;
        std::vector<std::string> tokens;
        _TODO_  // TODO: has to handle not only the string path tokens
        while( DictionaryParameter::pull_opt_path_token( path, current, index ) ) {
            tokens.push_back( current );
        }
        tokens.push_back( current );

        for( auto it = tokens.rbegin(); tokens.rend() != it; ++it ) {
            if( strcmp( it->c_str(), _top_as<NamedDict>(true).name() ) ) {
                emraise( assertFailed,
                        "Insertion proxy state check failed: current section is "
                        "named \"%s\" while \"%s\" expected (full path is %s).",
                        _top_as<NamedDict>(true).name(), current, name );
            }
            _pop();
        }
    } else {
        _pop();  // TODO: this line wasn't here... But it shall be, isn't it?
    }
    return *this;
}

LoDInsertionProxy
DictInsertionProxy::end_dict( const char * name ) {
    if( name && strcmp(name, _top_as<NamedDict>(true).name() ) ) {
        emraise( assertFailed
               , "Insertion proxy state check failed: current list is"
                 " named \"%s\" while \"%s\" expected."
               , _top_as<NamedDict>(true).name(), name );
    }
    _pop();
    return stack();
}

LoDInsertionProxy
DictInsertionProxy::bgn_list( const char * name, const char * description) {
    std::stack<InsertionTarget> s(stack());
    if( NULL != strchr(name, '.')
     || NULL != strchr(name, '[') || NULL != strchr(name, ']') ) {
        _DETAILED_TODO_( "Array argument name specification \"%s\""
                " given to insertion object contains path separator symbol"
                " (\".\") or array indexing expression (\"[\", \"]\"). List"
                " insertion proxy does not provide"
                " automatic dictionary creation.", name );
    }
    _TODO_  // TODO: further insertion code
    //InsertionProxyBase::LoS * los = new InsertionProxyBase::LoS( name, description );
    //s.top().dict().insert_parameter( los );
    //s.push( los );
    //return LoDInsertionProxy(s);
}

//
// List insertion proxy
// ///////////////////

DictInsertionProxy
LoDInsertionProxy::end_list( const char * listName ) {
    _TODO_  // TODO: further insertion code
    # if 0
    _stack.pop();
    if( _stack.top().is_list() ) {
        emraise( assertFailed
               , "Insertion proxy state check failed: stack top is not a dict"
                 " while en_list(name=\"%s\") invoked."
               , listName ? listName : "<null>" );
    }
    const char * realDictName = _stack.top().dict().name();
    if( listName && !strcmp(listName, realDictName ) ) {
        emraise( assertFailed
               , "bgn_list(name=\"%s\") doesn't match to enclosing"
                 " end_list(name=\"%s\")."
               , realDictName, listName );
    }
    return _stack;
    # endif
}

# if 0
DictInsertionProxy
LoDInsertionProxy::end_dict( ) {
    # if 0
    _stack.pop();
    if( _stack.top().is_list() ) {
        emraise( assertFailed
               , "Insertion proxy state check failed: stack top is not a dict"
                 " while en_list(name=\"%s\") invoked."
               , listName ? listName : "<null>" );
    }
    const char * realDictName = _stack.top().dict().name();
    if( listName && !strcmp(listName, realDictName ) ) {
        emraise( assertFailed
               , "bgn_list(name=\"%s\") doesn't match to enclosing"
                 " end_list(name=\"%s\")."
               , realDictName, listName );
    }
    return _stack;
    # endif
}
# endif

}  // namespace goo
}  // namespace dicts


