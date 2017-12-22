# include <cstring>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

namespace goo {
namespace dict {

InsertionProxy &
InsertionProxy::required_argument() {
    assert( !_stack.empty() );
    _stack.top()->_mark_last_inserted_as_required();
    return *this;
}

//InsertionProxy &
//InsertionProxy::as_flag() {
//    assert( !_stack.empty() );
//    _stack.top()->_set_is_flag_flag();
//    return *this;
//}

InsertionProxy::InsertionProxy( DictionaryParameter * root ) {
    _stack.push(root);
}

InsertionProxy &
InsertionProxy::bgn_sect( const char * name, const char * descr) {
    // Suppose, we have relative path and need to recursively insert
    // parent sections:
    char * path = strdupa( name ),
         * current = NULL;

    DictionaryParameter * newTop;

    while( DictionaryParameter::pull_opt_path_token( path, current ) ) {
        newTop = _stack.top()->probe_subsection( current );
        if( !newTop ) {
            newTop = new DictionaryParameter( current, nullptr );
            _stack.top()->insert_section( newTop );
        }
        _stack.push( newTop );
    }

    
    if( !(newTop = _stack.top()->probe_subsection( current )) ) {
        // Insert new section.
        newTop = new DictionaryParameter( current, descr );
        _stack.top()->insert_section( newTop );
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

InsertionProxy &
InsertionProxy::end_sect( const char * name ) {
    if( name ) {
        char * path = strdupa( name ),
             * current = NULL;
        std::vector<std::string> tokens;
        while( DictionaryParameter::pull_opt_path_token( path, current ) ) {
            tokens.push_back( current );
        }
        tokens.push_back( current );

        for( auto it = tokens.rbegin(); tokens.rend() != it; ++it ) {
            if( strcmp( it->c_str(), _stack.top()->name() ) ) {
                emraise( assertFailed,
                        "Insertion proxy state check failed: current section is "
                        "named \"%s\" while \"%s\" expected (full path is %s).",
                    _stack.top()->name(), current, name );
            }
            _stack.pop();
        }
    }
    return *this;
}

void
InsertionProxy::insert_copy_of( const iSingularParameter & sp,
                                const char * newName ) {
    iSingularParameter * isp =
                    clone_as<iAbstractParameter, iSingularParameter>( &sp );
    if( !! newName ) {
        isp->name( newName );
    }
    _stack.top()->insert_parameter( isp );
}

}  // namespace goo
}  // namespace dicts


