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

# include <cstring>
# include "goo_dict/types.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"
# include "goo_dict/appCfg/configuration.hpp"

namespace goo {
namespace dict {

//typedef  AppConfInsertionProxy;

InsertionProxy<String>::InsertionProxy( Subsection & d
                                           , const char * name )
    : AppConfTraits::IndexBy<String>::Dictionary::BaseInsertionProxy<InsertableParameter >(d)
    , _root(nullptr)
    , _latestInsertedRequired(nullptr) {
    _stack.push( std::make_pair(name, &d) );
}

InsertionProxy<String>::InsertionProxy( Configuration * R )
    : AppConfTraits::IndexBy<String>::Dictionary::BaseInsertionProxy<InsertableParameter >(*R)
    , _root(R)
    , _latestInsertedRequired(nullptr) {
    // ...
}

InsertionProxy<String>::InsertionProxy( Configuration * R, InsertionTargetsStack & s )
    : AppConfTraits::IndexBy<String>::Dictionary::BaseInsertionProxy<InsertableParameter >(*R)
    , _root(R)
    , _stack(s)
    , _latestInsertedRequired(nullptr) {
    // ...
}

InsertionProxy<String>::Subsection &
InsertionProxy<String>::_top() {
    if( _stack.empty() ) {
        if( !_root ) {
            emraise( badState, "Empty insertion proxy object." );
        }
        return *_root;
    }
    return *_stack.top().second;
}

void
InsertionProxy<String>::_index_by_shortcut( char shrtc, VBase * p ) {
    if( !_root ) {
        emraise( badState, "Unable to index parameter %p by shortcut %c with"
                " the insertion proxy object %p since proxy has no associated"
                " Configuration instance.", p, shrtc, this );
    }
    _root->_add_shortcut( shrtc, static_cast<Configuration::FeaturedBase *>(p) );
}

InsertionProxy<String>::Self &
InsertionProxy<String>::bgn_sect( const char * name
                                , const char * description ) {
    auto sPtr = _alloc<AppConfNameIndex>( std::make_tuple( _alloc<aspects::Description>(description) ) );
    emplace_subsection_copy( *_stack.top().second, name, sPtr );
    _stack.push( std::pair<std::string, Subsection *>(name, sPtr) );
    return *this;
}

void
InsertionProxy<String>::emplace_subsection_copy( Subsection & self
                                               , const char * name
                                               , Subsection * sPtr) {
    auto ir = self.emplace( name, sPtr );
    if( !ir.second ) {
        emraise( nonUniq, "Unable to insert new section named \"%s\" since"
            " there is one with same name (%p).", name, ir.first->second );
    }
}

InsertionProxy<String>::Self &
InsertionProxy<String>::end_sect( const char * name ) {
    if( name && '\0' != name[0] ) {
        if( name != _stack.top().first ) {
            emraise(assertFailed, "end_sect(\"%s\") does not correspond to"
                    " bgn_sect(\"%s\").", name, _stack.top().first.c_str() );
        }
    }
    _stack.pop();
    return *this;
}

InsertionProxy<String>::Self &
InsertionProxy<String>::flag( const char * name
                            , const char * description ) {
    assert( name && '\0' != name[0] );
    return flag( '\0', name, description );
}

InsertionProxy<String>::Self &
InsertionProxy<String>::flag( char shortcut
                            , const char * name
                            , const char * description ) {
    auto pPtr = _alloc_parameter<bool>( _alloc<aspects::Description>(description)
                                      , _alloc<aspects::TStringConvertible<bool, _Goo_m_VART_LIST_APP_CONF>>()
                                      , _alloc<aspects::CharShortcut>(shortcut)
                                      , _latestInsertedRequired = _alloc<aspects::ImplicitValue<bool, _Goo_m_VART_LIST_APP_CONF>>()
                                      , _alloc<aspects::IsSet>()
                                      , _alloc<aspects::Array>(false)
                                      );
    if( name && '\0' != name[0] ) {
        _insert_parameter( name, pPtr );
    }
    if( '\0' != shortcut ) {
        assert( isalnum(shortcut) );
        _index_by_shortcut( shortcut, pPtr );
    }
    # ifndef NDEBUG
    else { assert( name && '\0' != name[0] ); }
    # endif
    _latestInsertedRequired->set_required(false);
    _latestInsertedRequired->set_expects_argument(false);
    _latestInsertedRequired->set_being_implicit(true);
    static_cast<aspects::ImplicitValue<bool, _Goo_m_VART_LIST_APP_CONF>*>(_latestInsertedRequired)
            ->set_implicit_value(true);
    _latestInsertedRequired = nullptr;
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


