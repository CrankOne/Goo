# include <cstring>
#include <goo_dict/types.hpp>

# include "goo_dict/appCfg/insertion_proxy.tcc"
//# include "goo_dict/configuration.hpp"

namespace goo {
namespace dict {

AppConfInsertionProxy::Subsection &
AppConfInsertionProxy::_top() {
    if( _stack.empty() ) {
        if( !_root ) {
            emraise( badState, "Empty insertion proxy object." );
        }
        _TODO_  // TODO: return *_root;
    }
    return *_stack.top().second;
}

void
AppConfInsertionProxy::_index_by_shortcut( char shrtc, VBase * p ) {
    if( !_root ) {
        emraise( badState, "Unable to index parameter %p by shortcut %c with"
                " the insertion proxy object %p since proxy has no associated"
                " Configuration instance.", p, shrtc, this );
    }
    _TODO_   // TODO: _root->emplace( shrtc, p );
}

AppConfInsertionProxy::Self &
AppConfInsertionProxy::bgn_sect( const std::string & name
                              , const std::string & description ) {
    auto sPtr = new AppConfNameIndex( description );
    auto ir = _stack.top().second->emplace( name, sPtr );
    if( !ir.second ) {
        emraise( nonUniq, "Unable to insert new section named \"%s\" since"
            " there is one with same name (%p).", name.c_str(), ir.first->second );
    }
    _stack.push( std::pair<std::string, Subsection *>(name, sPtr) );
    return *this;
}

AppConfInsertionProxy::Self &
AppConfInsertionProxy::end_sect( const std::string & name ) {
    if( !name.empty() ) {
        if( name != _stack.top().first ) {
            emraise(assertFailed, "end_sect(\"%s\") does not correspond to"
                    " bgn_sect(\"%s\").", name.c_str(), _stack.top().first.c_str() );
        }
    }
    _stack.pop();
    return *this;
}

}  // namespace goo
}  // namespace dicts


