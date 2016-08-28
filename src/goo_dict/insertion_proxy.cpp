# include <cstring>

# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/dict.hpp"

namespace goo {
namespace dict {

InsertionProxy &
InsertionProxy::required_argument() {
    assert( !_stack.empty() );
    _stack.top()->_set_is_argument_required_flag();
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
    auto newDict = new Dictionary( name, descr );
    _stack.top()->insert_section( newDict );
    _stack.push( newDict );
    return *this;
}

InsertionProxy &
InsertionProxy::end_sect( const char * name ) {
    if( name ) {
        if( !strcmp( name, _stack.top()->name() ) ) {
            emraise( assertFailed,
                    "Insertion proxy state check failed: current section is named \"%s\" while \"%s\" expected.",
                    _stack.top()->name(),
                    name );
        }
    }
    _stack.pop();
    return *this;
}

}  // namespace goo
}  // namespace dicts


