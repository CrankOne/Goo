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

# include <typeindex>
# include <functional>
# include <unordered_map>
# include <cstring>

# include "goo_exception.hpp"

# include "goo_dict/types.hpp"
# include "goo_dict/parameter_abstract.hpp"
# include "goo_dict/parameter_values.hpp"
# include "goo_mixins/vcopy.tcc"

namespace goo {
namespace dict {

/**@brief Type traits template defining general properties of parameter mapping.
 * */
template< typename KeyT
        , typename ValueT
        , bool hasNameFieldT=std::is_base_of<AbstractParameter, ValueT>::value > struct IndexingTraits {
    typedef KeyT Key;
    typedef ValueT Value;
    typedef Value * ValueHandle;
    typedef const Value * ConstValueHandle;
    typedef Hash<Key, ValueHandle> IndexingContainer;
    typedef typename IndexingContainer::iterator Iterator;
    typedef typename IndexingContainer::const_iterator ConstIterator;
    typedef std::pair<Iterator, bool> InsertionResult;

    static ValueHandle dereference_iterator( Iterator it )
        { return it->second; }
    static ConstValueHandle dereference_iterator( ConstIterator it )
        { return it->second; }
    static Iterator find_item( const Key & k, IndexingContainer & c ) {
        return c.find( k );
    }
    static ConstIterator find_item( const Key & k, const IndexingContainer & c ) {
        return c.find( k );
    }
    static InsertionResult insert_item( const Key & k, ValueHandle v, IndexingContainer & c) {
        return c.emplace( k, v );
    }
};

template<> struct IndexingTraits<void, iBaseValue, false> {
    typedef iBaseValue * Key;
    typedef iBaseValue Value;
    typedef Value * ValueHandle;
    typedef const Value * ConstValueHandle;
    typedef Set<ValueHandle> IndexingContainer;
    typedef typename IndexingContainer::iterator Iterator;
    typedef typename IndexingContainer::const_iterator ConstIterator;
    typedef std::pair<Iterator, bool> InsertionResult;

    static ValueHandle dereference_iterator( Iterator it ) {
        return *it;
    }
    static ConstValueHandle dereference_iterator( ConstIterator it ) {
        return *it;
    }
    static Iterator find_item( const Key & k, IndexingContainer & c ) {
        return c.find( k );
    }
    static ConstIterator find_item( const Key & k, const IndexingContainer & c ) {
        return c.find( k );
    }
    static InsertionResult insert_item( const Key & k, ValueHandle v, IndexingContainer & c) {
        return c.emplace( v );
    }
};

/// In order to avoid duplication of the string name data, the indexing traits
/// provides customized key-hashing behaviour.
template<typename ValueT> struct IndexingTraits<std::string, ValueT, true> {
    typedef std::string Key;
    typedef ValueT Value;
    typedef Value * ValueHandle;
    typedef const Value * ConstValueHandle;
    typedef std::unordered_multimap<std::hash<std::string>::result_type, ValueHandle> IndexingContainer;
    typedef typename IndexingContainer::iterator Iterator;
    typedef typename IndexingContainer::const_iterator ConstIterator;
    typedef std::pair<Iterator, bool> InsertionResult;

    static ValueHandle dereference_iterator( Iterator it )
        { return it->second; }
    static ConstValueHandle dereference_iterator( ConstIterator it )
        { return it->second; }
    static Iterator find_item( const Key & k, IndexingContainer & c ) {
        auto cIt = find_item(k, (const IndexingContainer &) c );
        // Iterator const-cast trick:
        // https://stackoverflow.com/a/10669041/1734499
        return c.erase( cIt );
    }
    static ConstIterator find_item( const Key & k, const IndexingContainer & c ) {
        std::hash<std::string> hash_fn;
        std::hash<std::string>::result_type hashCode = hash_fn(k);
        auto collidingEls = c.equal_range(hashCode);
        for( auto it = collidingEls.first; it != collidingEls.second; ++it ) {
            if( !strcmp( it->second->name(), k.c_str() ) ) {
                return it;
            }
        }
        return c.end();
    }
    static InsertionResult insert_item( const Key & k, ValueHandle v, IndexingContainer & c) {
        std::hash<std::string> hash_fn;
        std::hash<std::string>::result_type hashCode = hash_fn(k);
        if( !c.count( hashCode ) ) {
            return std::make_pair(c.emplace(hashCode, v), true);
        } else {
            auto collidingEls = c.equal_range(hashCode);
            // Check colliding elements:
            for( auto it = collidingEls.first; it != collidingEls.second; ++it ) {
                if( !strcmp( it->second->name(), k.c_str() ) ) {
                    emraise( nonUniq, "Duplicating key insertion: \"%s\"."
                           , k.c_str() );
                }
            }
            return std::make_pair(c.emplace(hashCode, v), true);
        }
    }
};

template< typename KeyT
        , typename ValueT
        , bool hasNameFieldT=std::is_base_of<AbstractParameter, ValueT>::value>
class iDictionaryIndex : private IndexingTraits<KeyT, ValueT, hasNameFieldT>::IndexingContainer {
public:
    typedef iDictionaryIndex<KeyT, ValueT, hasNameFieldT> Self;
    typedef IndexingTraits<KeyT, ValueT, hasNameFieldT> Traits;
    typedef typename Traits::Key Key;
    typedef typename Traits::Value Value;

    typedef typename Traits::ValueHandle ValueHandle;
    typedef typename Traits::ConstValueHandle ConstValueHandle;
    typedef typename Traits::IndexingContainer IndexingContainer;
    typedef typename Traits::Iterator Iterator;
    typedef typename Traits::ConstIterator ConstIterator;
    typedef typename Traits::InsertionResult InsertionResult;
protected:
    virtual Iterator _V_find_item( const Key & k ) {
        return Traits::find_item( k, *this );
    }
    virtual ConstIterator _V_find_item( const Key & k ) const {
        return Traits::find_item( k, *this );
    }
    virtual bool _V_iterator_valid( Iterator it ) const {
        return it != IndexingContainer::end();
    }
    virtual bool _V_iterator_valid( ConstIterator it ) const {
        return it != IndexingContainer::end();
    }
    virtual InsertionResult _V_insert_item( const Key & k, ValueHandle v ) {
        return Traits::insert_item( k, v, *this );
    }
    virtual void _V_remove_item( Iterator it ) {
        IndexingContainer::erase( it );
    }
public:
    virtual ~iDictionaryIndex() {}

    /// Returns a pointer to an item indexed by given key (const variant).
    /// Returns nullptr if item does not exist.
    virtual ConstValueHandle item_ptr( const Key & k ) const {
        ConstIterator it = find_item( k );
        if( !iterator_valid( it ) ) {
            return nullptr;
        }
        return Traits::dereference_iterator( it );
    }

    /// Returns a pointer to an item indexed by given key. Returns nullptr
    /// if item does not exist.
    virtual ValueHandle item_ptr( const Key & k ) {
        const Self * cThis = this;
        return const_cast<ValueHandle>(cThis->item_ptr(k));
    }

    /// Inserts new entry with given indexing key.
    virtual InsertionResult insert_item( const Key & k, ValueHandle vPtr ) {
        return _V_insert_item( k, vPtr );
    }

    /// Removes entry (referenced by given iterator) from index.
    virtual void remove_item( Iterator it ) {
        _V_remove_item( it );
    }

    /// Returns iterator to an item within given key. If no entry found,
    /// returned iterator will be "invalid" (check with iterator_valid()
    /// method).
    virtual Iterator find_item( const Key & k ) {
        return _V_find_item(k);
    }

    /// Returns iterator to an item within given key. If no entry found,
    /// returned iterator will be "invalid" (check with iterator_valid()
    /// method) (const variant).
    virtual ConstIterator find_item( const Key & k ) const {
        return _V_find_item(k);
    }

    /// Returns true if entry by given key exists.
    virtual bool has_item( const Key & k ) const {
        return (bool) item_ptr( k );
    }

    /// Returns true if iterator refers to an existing entry.
    virtual bool iterator_valid( Iterator it ) const {
        return _V_iterator_valid( it );
    }

    /// Returns true if const iterator refers to an existing entry.
    virtual bool iterator_valid( ConstIterator it ) const {
        return _V_iterator_valid( it );
    }

    /// Abstract base data pointer querying method implementation. Intended
    /// for debug use only.
    virtual typename Traits::IndexingContainer & container_ref() {
        return *static_cast<typename Traits::IndexingContainer *>(this);
    };

    /// Abstract base data pointer querying method implementation. Intended
    /// for debug use only (const version).
    virtual const typename Traits::IndexingContainer & container_const_ref() const {
        return *static_cast<const typename Traits::IndexingContainer *>(this);
    };
};  // class iDictionaryIndex

template<typename KeyT, typename ValueT, bool hasNameFieldT=std::is_base_of<AbstractParameter, ValueT>::value>
class DictionaryIndex;

# define _Goo_m_for_each_index_method( m ) \
    m( item_ptr )       \
    m( insert_item )    \
    m( remove_item )    \
    m( find_item )      \
    m( has_item )       \
    m( iterator_valid )

template<typename KeyT, typename ValueT, bool hasNameFieldT>
class DictionaryIndex : public iDictionaryIndex<KeyT, ValueT, hasNameFieldT>
                      , protected virtual iDictionaryIndex<void, iBaseValue, false> {
public:
    typedef iDictionaryIndex<void, iBaseValue, false> PhysicalContainer;
    typedef iDictionaryIndex<KeyT, ValueT, hasNameFieldT> Parent;

    using Parent::Traits;

    # define _Goo_m_import_parent_methods( mNameM ) using Parent:: mNameM;
    _Goo_m_for_each_index_method( _Goo_m_import_parent_methods )
    # undef _Goo_m_import_parent_methods
protected:
    virtual typename Parent::InsertionResult
                _V_insert_item( const typename Parent::Key & k
                              , typename Parent::ValueHandle v ) override {
        typename Parent::InsertionResult ir = Parent::insert_item( k, v );
        if( ir.second ) {
            auto it = PhysicalContainer::find_item( v );
            if( !PhysicalContainer::iterator_valid(it) ) {
                PhysicalContainer::insert_item( v, v );
            }
        }
        return ir;
    }
    virtual void _V_remove_item( typename Parent::Iterator it ) override {
        _TODO_  // TODO: attempt to remove from physical container as well
        Parent::remove_item( it );
    }
};

# undef _Goo_m_for_each_index_method

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_PLURAL_H

