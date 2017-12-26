# include <cstring>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

namespace goo {
namespace dict {

DictionaryParameter &
DictInsertionProxy::InsertionTarget::dict() {
    if( !_isDict ) {
        emraise( badState, "Latest object (by address %p) in parameter "
                " insertion proxy stack refers to list instead of dictionary"
                " while dictionary requested.", this );
    }
    return *_dPtr;
}

DictInsertionProxy::InsertionTarget::LOS &
DictInsertionProxy::InsertionTarget::list() {
     if( ! _losPtr ) {
        emraise( badState, "Latest object (by address %p) in parameter "
                " insertion proxy stack refers to dictionary instead of "
                " list while list requested.", this );
    }
    return *_losPtr;
}

DictInsertionProxy &
DictInsertionProxy::required_argument() {
    assert( !_stack.empty() );
    _stack.top().dict()._mark_last_inserted_as_required();
    return *this;
}

//DictInsertionProxy &
//DictInsertionProxy::as_flag() {
//    assert( !_stack.empty() );
//    _stack.top()->_set_is_flag_flag();
//    return *this;
//}

DictInsertionProxy::DictInsertionProxy( DictionaryParameter * root ) {
    _stack.push(root);
}

DictInsertionProxy &
DictInsertionProxy::bgn_sect( const char * name, const char * descr) {
    // Suppose, we have relative path and need to recursively insert
    // parent sections:
    char * path = strdupa( name ),
         * current = NULL;

    DictionaryParameter * newTop;

    while( DictionaryParameter::pull_opt_path_token( path, current ) ) {
        newTop = _stack.top().dict().probe_subsection( current );
        if( !newTop ) {
            newTop = new DictionaryParameter( current, nullptr );
            _stack.top().dict().insert_section( newTop );
        }
        _stack.push( newTop );
    }
    
    if( !(newTop = _stack.top().dict().probe_subsection( current )) ) {
        // Insert new section.
        newTop = new DictionaryParameter( current, descr );
        _stack.top().dict().insert_section( newTop );
        _stack.push( newTop );
    } else {
        // Append existing section:
        _stack.push( newTop );
        if( !newTop->description() ) {
            newTop->_append_description( descr );
        }
    }

    return *this;
}

DictInsertionProxy &
DictInsertionProxy::end_sect( const char * name ) {
    if( name ) {
        char * path = strdupa( name ),
             * current = NULL;
        std::vector<std::string> tokens;
        while( DictionaryParameter::pull_opt_path_token( path, current ) ) {
            tokens.push_back( current );
        }
        tokens.push_back( current );

        for( auto it = tokens.rbegin(); tokens.rend() != it; ++it ) {
            if( strcmp( it->c_str(), _stack.top().dict().name() ) ) {
                emraise( assertFailed,
                        "Insertion proxy state check failed: current section is "
                        "named \"%s\" while \"%s\" expected (full path is %s).",
                    _stack.top().dict().name(), current, name );
            }
            _stack.pop();
        }
    }
    return *this;
}

void
DictInsertionProxy::insert_copy_of( const iSingularParameter & sp,
                                const char * newName ) {
    iSingularParameter * isp =
                    clone_as<iAbstractParameter, iSingularParameter>( &sp );
    if( !! newName ) {
        isp->name( newName );
    }
    _stack.top().dict().insert_parameter( isp );
}

# if 0
LoDInsertionProxy
DictInsertionProxy::bgn_list( const char *, const char * ) {
    _TODO_
}
# endif


}  // namespace goo
}  // namespace dicts


