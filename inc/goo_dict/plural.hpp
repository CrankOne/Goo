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

# ifndef H_GOO_PARAMETERS_PLURAL_H
# define H_GOO_PARAMETERS_PLURAL_H

# include "goo_exception.hpp"

# include "goo_dict/types.hpp"
# include "goo_dict/parameter_abstract.hpp"
# include "goo_dict/parameter_values.hpp"
# include "goo_mixins/vcopy.tcc"

namespace goo {
namespace dict {

template< typename KeyT
        , typename ValueT> struct IndexingTraits {
    typedef KeyT Key;
    typedef ValueT Value;
    typedef Value * ValueHandle;
    typedef const Value * ConstValueHandle;
    typedef Hash<Key, ValueHandle> IndexingContainer;
    typedef typename IndexingContainer::iterator Iterator;
    typedef typename IndexingContainer::const_iterator ConstIterator;

    static ValueHandle dereference_iterator( Iterator it )
        { return it->second; }
    static ConstValueHandle dereference_iterator( ConstIterator it )
        { return it->second; }
};


template<> struct IndexingTraits<ListIndex, iBaseValue> {
    typedef ListIndex Key;
    typedef iBaseValue Value;
    typedef iBaseValue * ValueHandle;
    typedef const iBaseValue * ConstValueHandle;
    typedef List<Value *> IndexingContainer;
    typedef typename IndexingContainer::iterator Iterator;
    typedef typename IndexingContainer::const_iterator ConstIterator;

    static ValueHandle dereference_iterator( Iterator it )
        { return *it; }
    static ConstValueHandle dereference_iterator( ConstIterator it )
        { return *it; }
};

template< typename KeyT
        , typename ValueT>
class iDictionaryIndex {
public:
    typedef KeyT Key;
    typedef ValueT Value;
    typedef iDictionaryIndex<Key, Value> Self;
    typedef IndexingTraits<Key, Value> Traits;

    typedef typename Traits::ValueHandle ValueHandle;
    typedef typename Traits::ConstValueHandle ConstValueHandle;
    typedef typename Traits::IndexingContainer IndexingContainer;
    typedef typename Traits::Iterator Iterator;
    typedef typename Traits::ConstIterator ConstIterator;
protected:
    virtual Iterator _V_find_item( const Key & ) = 0;
    virtual ConstIterator _V_find_item( const Key & ) const = 0;
    virtual bool _V_iterator_valid( Iterator ) const = 0;
    virtual bool _V_iterator_valid( ConstIterator ) const = 0;
    virtual Iterator _V_insert_item( const Key &, ValueHandle ) = 0;
    virtual void _V_remove_item( Iterator ) = 0;

    virtual ConstValueHandle _V_get_item( const Key & k ) const {
        auto it = find_item( k );
        if( iterator_valid( it ) ) {
            return Traits::dereference_iterator(it);
        }
        _TODO_   // TODO: emraise( notFound )
    }
    virtual ValueHandle _V_get_item( const Key & k ) {
        const Self * cThis = this;
        return const_cast<Value *>(cThis->_V_get_item(k));
    }
public:
    virtual ~iDictionaryIndex() {}
    virtual ValueT & item( const KeyT & k )
                { return *_V_get_item( k ); }
    virtual const ValueT & item( const KeyT & k ) const
                { return *_V_get_item( k ); }
    virtual Iterator insert_item( const Key & k, Value * vPtr )
                { return _V_insert_item(k, vPtr); }
    virtual void remove_item( Iterator it ) 
                { _V_remove_item( it ); }
    virtual Iterator find_item( const Key & k )
                { return _V_find_item(k); }
    virtual ConstIterator find_item( const Key & k ) const
                { return _V_find_item(k); }
    virtual bool has_item( const Key & k ) const
                { return !! _V_get_item( k ); }
    virtual bool iterator_valid( Iterator it ) const
                { return _V_iterator_valid( it ); }
    virtual bool iterator_valid( ConstIterator it ) const
                { return _V_iterator_valid( it ); }
};  // class iDictionaryIndex

template<typename KeyT, typename ValueT>
class DictionaryIndex;

/// A special case --- integral index.
template<>
class DictionaryIndex<ListIndex, iBaseValue> : public iDictionaryIndex<ListIndex, iBaseValue>
                                             , public List<iBaseValue *> {
protected:
    virtual Iterator _V_find_item( const Key & ) override;
    virtual ConstIterator _V_find_item( const Key & ) const override;
    virtual bool _V_iterator_valid( Iterator ) const override;
    virtual bool _V_iterator_valid( ConstIterator ) const override;
    virtual Iterator _V_insert_item( const Key &, ValueHandle ) override;
    virtual void _V_remove_item( Iterator ) override;
};

template<typename KeyT, typename ValueT>
class DictionaryIndex : protected virtual DictionaryIndex<ListIndex, iBaseValue>
                      , public iDictionaryIndex<KeyT, ValueT>
                      , public Hash<KeyT, ValueT *> {
public:
    typedef KeyT Key;
    typedef ValueT Value;
    typedef iDictionaryIndex<Key, Value> Parent;
    typedef typename Parent::Traits Traits;

    typedef typename Traits::ValueHandle ValueHandle;
    typedef typename Traits::ConstValueHandle ConstValueHandle;
    typedef typename Traits::IndexingContainer IndexingContainer;
    typedef typename Traits::Iterator Iterator;
    typedef typename Traits::ConstIterator ConstIterator;
protected:
    virtual const Value * _V_get_item( const Key & ) const override;
    virtual Value * _V_get_item( const Key & k ) override;
    virtual Iterator _V_insert_item( const Key &, Value * ) override;
    virtual void _V_remove_item( Iterator ) override;
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_PLURAL_H

