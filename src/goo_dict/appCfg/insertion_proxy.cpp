# include <cstring>
# include "goo_dict/types.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"
# include "goo_dict/appCfg/configuration.hpp"

namespace goo {
namespace dict {

//typedef  AppConfInsertionProxy;

InsertionProxy<std::string>::InsertionProxy(Subsection & d, const std::string & name)
    : AppConfTraits::IndexBy<std::string>::Dictionary::BaseInsertionProxy<InsertableParameter >(d)
    , _root(nullptr)
    , _latestInsertedRequired(nullptr) {
    _stack.push( std::make_pair(name, &d) );
}

InsertionProxy<std::string>::InsertionProxy( Configuration * R )
    : AppConfTraits::IndexBy<std::string>::Dictionary::BaseInsertionProxy<InsertableParameter >(*R)
    , _root(R)
    , _latestInsertedRequired(nullptr) {
    // ...
}

InsertionProxy<std::string>::InsertionProxy( Configuration * R, InsertionTargetsStack & s )
    : AppConfTraits::IndexBy<std::string>::Dictionary::BaseInsertionProxy<InsertableParameter >(*R)
    , _root(R)
    , _stack(s)
    , _latestInsertedRequired(nullptr) {
    // ...
}

InsertionProxy<std::string>::Subsection &
InsertionProxy<std::string>::_top() {
    if( _stack.empty() ) {
        if( !_root ) {
            emraise( badState, "Empty insertion proxy object." );
        }
        return *_root;
    }
    return *_stack.top().second;
}

void
InsertionProxy<std::string>::_index_by_shortcut( char shrtc, VBase * p ) {
    if( !_root ) {
        emraise( badState, "Unable to index parameter %p by shortcut %c with"
                " the insertion proxy object %p since proxy has no associated"
                " Configuration instance.", p, shrtc, this );
    }
    _root->_add_shortcut( shrtc, static_cast<Configuration::FeaturedBase *>(p) );
}

InsertionProxy<std::string>::Self &
InsertionProxy<std::string>::bgn_sect( const std::string & name
                                     , const std::string & description ) {
    auto sPtr = _alloc<AppConfNameIndex>( std::make_tuple( _alloc<aspects::Description>(description) ) );
    emplace_subsection_copy( *_stack.top().second, name, sPtr );
    _stack.push( std::pair<std::string, Subsection *>(name, sPtr) );
    return *this;
}

void
InsertionProxy<std::string>::emplace_subsection_copy( Subsection & self
                                                  , const std::string & name
                                                  , Subsection * sPtr) {
    auto ir = self.emplace( name, sPtr );
    if( !ir.second ) {
        emraise( nonUniq, "Unable to insert new section named \"%s\" since"
            " there is one with same name (%p).", name.c_str(), ir.first->second );
    }
}

InsertionProxy<std::string>::Self &
InsertionProxy<std::string>::end_sect( const std::string & name ) {
    if( !name.empty() ) {
        if( name != _stack.top().first ) {
            emraise(assertFailed, "end_sect(\"%s\") does not correspond to"
                    " bgn_sect(\"%s\").", name.c_str(), _stack.top().first.c_str() );
        }
    }
    _stack.pop();
    return *this;
}

InsertionProxy<char>::InsertionProxy( AppConfTraits::template IndexBy<char>::Dictionary & d )
    : TheDictionary::BaseInsertionProxy<InsertableParameter >(d) {}

std::pair<typename AppConfTraits::template IndexBy<char>::DictValue::Value::iterator, bool>
InsertionProxy<char>::insert( char c, AppConfTraits::VBase * e ) {
    return _insert_parameter( c, static_cast<AppConfTraits::FeaturedBase *>(e) );
}

}  // namespace goo
}  // namespace dicts


