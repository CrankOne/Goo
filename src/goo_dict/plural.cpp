/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# include "goo_dict/plural.hpp"

namespace goo {
namespace dict {

// A special case --- "indexed" list (merely a restrictive wrapper to STL's 
// list container) implementing a rudimentary "dictionary" interface.

DictionaryIndex<ListIndex, iBaseValue>::Iterator
DictionaryIndex<ListIndex, iBaseValue>::_V_find_item( const Key & k ) {
    const DictionaryIndex<ListIndex, iBaseValue> * cThis = this;
    auto cIt = cThis->_V_find_item( k );
    // Trick below. See: https://stackoverflow.com/a/10669041/1734499
    return DictionaryIndex<ListIndex, iBaseValue>::erase( cIt, cIt );
}

DictionaryIndex<ListIndex, iBaseValue>::ConstIterator
DictionaryIndex<ListIndex, iBaseValue>::_V_find_item( const Key & k ) const {
    if( k >= List<iBaseValue *>::size() ) {
        emraise( overflow, "Index %ld refers to non-existing element."
                " List contains %zu items only.", k, List<iBaseValue *>::size() );
    }
    ListIndex li = 0;
    List<iBaseValue *>::const_iterator it;
    for( it = List<iBaseValue *>::begin()
       ; li < k 
       ; ++li, ++it ) {}
    return it;
}

bool
DictionaryIndex<ListIndex, iBaseValue>::_V_iterator_valid( Iterator it ) const {
    return it != DictionaryIndex<ListIndex, iBaseValue>::end();
}

bool
DictionaryIndex<ListIndex, iBaseValue>::_V_iterator_valid( ConstIterator it ) const {
    return it != DictionaryIndex<ListIndex, iBaseValue>::end();
}

DictionaryIndex<ListIndex, iBaseValue>::Iterator
DictionaryIndex<ListIndex, iBaseValue>::_V_insert_item( const Key & k, ValueHandle v ) {
    if( k != DictionaryIndex<ListIndex, iBaseValue>::size() ) {
        emraise( badState, "Items in list may be only inserted into its end."
                " %p has %zu elements, while insertion of %ld-th element"
                " has been invoked."
                , this, DictionaryIndex<ListIndex, iBaseValue>::size(), k )
    }
    DictionaryIndex<ListIndex, iBaseValue>::push_back( v );
    return -- (DictionaryIndex<ListIndex, iBaseValue>::end());
}

void
DictionaryIndex<ListIndex, iBaseValue>::_V_remove_item( Iterator it ) {
    DictionaryIndex<ListIndex, iBaseValue>::erase( it );
}

}  // namespace dict
}  // namespace goo

