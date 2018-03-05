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

# include "goo_dict/common_aspects.hpp"
# include "goo_dict/types.hpp"
# include "goo_dict/appCfg/traits.hpp"
# include "goo_dict/appCfg/insertion_proxy.tcc"

namespace goo {
namespace dict {

void
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::copy_dict_entry(
                    typename DictValue::Value::iterator it
                  , Dictionary * /*dct*/ ) {
    // TODO: use allocator submitted by dict instance here
    //Dictionary::BaseInsertionProxy<InsertableParameter> ip( *dct );
    it->second = goo::clone_as< iAbstractValue
                              , FeaturedBase
                              , FeaturedBase >( it->second );
}

// generic options container

InsertionProxy<String>
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::insertion_proxy() {
    return InsertionProxy<String>(
            dynamic_cast<InsertionProxy<String>::Subsection &>(*this) );
}

Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::Aspect( const Aspect & o
                                                                  , Dictionary * this_ ) {
    // iterate among subsections and copy them recursively.
    for( auto it = o.subsections().begin()
            ; o.subsections().end() != it
            ; ++it ) {
        // TODO: use this_'s allocator here:
        auto subsectCopy = goo::clone_as< iAbstractValue
                                        , Dictionary
                                        , Dictionary>( it->second );
        InsertionProxy<String>::emplace_subsection_copy( *this_
                                                       , it->first
                                                       , subsectCopy );
    }
}

const Traits<_Goo_m_VART_LIST_APP_CONF>::FeaturedBase &
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::operator[]( const String & path ) const {
    std::vector< char, TheAllocatorHandle<char> > namecache;
    return operator[]( utils::dpath( path, namecache ).front() );
}

/// Returns a parameter entry by given path expression (mutable).
Traits<_Goo_m_VART_LIST_APP_CONF>::FeaturedBase &
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::operator[]( const String & path ) {
    std::vector<char, TheAllocatorHandle<char> > namecache;
    return operator[]( utils::dpath( path, namecache ).front() );
}

/// Returns a parameter entry by given path (const).
const Traits<_Goo_m_VART_LIST_APP_CONF>::FeaturedBase &
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::operator[]( const utils::DictPath & dp ) const {
    if( dp.isIndex ) {
        emraise( badParameter, "Current path token is an integer index."
            " Unable to dereference it within application configuration"
            " context." );
    }
    if( dp.next ) {
        return subsection_ptr( dp.id.name )->operator[](*dp.next);
    }
    // that's a terminating path token:
    # ifdef NDEBUG
    return static_cast<const Subsection *>(this)->entry(dp.id.name);
    # else
    auto downCastedPtr = dynamic_cast<const Subsection *>(this);
    assert( downCastedPtr );  // failed, indicates broken inheritance
    return downCastedPtr->entry(dp.id.name);
    # endif
}

/// Returns a parameter entry by given path (const).
Traits<_Goo_m_VART_LIST_APP_CONF>::FeaturedBase &
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<String>::Aspect::operator[]( const utils::DictPath & dp ) {
    if( dp.isIndex ) {
        emraise( badParameter, "Current path token is an integer index."
            " Unable to dereference it within application configuration"
            " context." );
    }
    if( dp.next ) {
        return subsection( dp.id.name )[*dp.next];
    }
    // that's a terminating path token:
    # ifdef NDEBUG
    return static_cast<Subsection *>(this)->entry(dp.id.name);
    # else
    auto downCastedPtr = dynamic_cast<Subsection *>(this);
    assert( downCastedPtr );  // failed, indicates broken inheritance
    return downCastedPtr->entry(dp.id.name);
    # endif
}

// shortcut-only options container

InsertionProxy<char>
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<char>::Aspect::insertion_proxy() {
    return InsertionProxy<char>(dynamic_cast<GenericDictionary<char, _Goo_m_VART_LIST_APP_CONF> &>(*this));
}

void
Traits<_Goo_m_VART_LIST_APP_CONF>::IndexBy<char>::copy_dict_entry(
                    typename DictValue::Value::iterator it
                  , Dictionary * dct ) {
    // TODO: use allocator submitted by dict instance here
    //Dictionary::BaseInsertionProxy<InsertableParameter> ip( *dct );
    it->second = goo::clone_as< iAbstractValue
                              , FeaturedBase
                              , FeaturedBase >( it->second );
}

}  // namespace dict
}  // namespace goo
