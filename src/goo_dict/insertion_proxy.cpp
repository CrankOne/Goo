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

InsertionProxy::InsertionProxy( Dictionary * root ) {
    _stack.push(root);
}

InsertionProxy &
InsertionProxy::bgn_sect( const char * name, const char * descr) {
    // Suppose, we have relative path and need to recursively insert
    // parent sections:
    char * path = strdupa( name ),
         * current = NULL;

    Dictionary * newTop;

    while( Dictionary::pull_opt_path_token( path, current ) ) {
        newTop = _stack.top()->probe_subsection( current );
        if( !newTop ) {
            newTop = new Dictionary( current, nullptr );
            _stack.top()->insert_section( newTop );
        }
        _stack.push( newTop );
    }

    
    if( !(newTop = _stack.top()->probe_subsection( current )) ) {
        // Insert new section.
        newTop = new Dictionary( current, descr );
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
        while( Dictionary::pull_opt_path_token( path, current ) ) {
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
InsertionProxy::insert_copy_of( const iSingularParameter & sp ) {
    _stack.top()->insert_parameter( clone_as<iAbstractParameter, iSingularParameter>( &sp ) );
}

}  // namespace goo
}  // namespace dicts


