# include <cstring>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

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

InsertionProxyBase::InsertionTargetsStack
InsertionProxyBase::combine_path( const InsertionTargetsStack & currentStack
                                , const char * path_
                                , bool extend
                                , const std::string ed) {
    // Suppose, we have relative path and need to recursively insert
    // parent sections:
    char * path = strdupa( path_ ),
         * current = NULL;

    DictionaryParameter * newTop;

    InsertionProxyBase h(currentStack);

    long index;
    int rc;
    while(0x2 & (rc = DictionaryParameter::pull_opt_path_token( path, current, index ))) {
        if( !(rc & 0x1) ) {
            // Extracted token refers to a named parameter
            newTop = h._top_as<Dict>(false).probe_subsection( current );
            if( !newTop ) {
                // Extracted token refers to non-existing dict.
                if( !extend ) {
                    emraise( notFound, "No subsection named \"%s\" found in"
                            " subsection or dictionary instance %p."
                           , current, &(h._top_as<Dict>(false)) );
                }
                // ... otherwise, extend
                newTop = new DictionaryParameter( current, ed.c_str() );
                h._top_as<Dict>(false).insert_section( newTop );
            }
            h._push_dict( newTop );
        } else {
            // Extracted token refers to a parameter with index and rc is its
            // index within list.
            _TODO_
        }
    }

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


