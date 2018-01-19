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
# include <sstream>

# include "goo_exception.hpp"

# include "goo_dict/types.hpp"
# include "goo_dict/parameter_values.hpp"
# include "goo_mixins/vcopy.tcc"
# include "goo_utility.hpp"

namespace goo {
namespace dict {

/// Indexing traits defines, for given value-keeper base and supplementary info
/// type, the particular indexing features.
template< typename BVlT
        , template <class> class TVlT
        , class ... SuppInfoTs > struct IndexingTraits;

template< typename KeyT
        , typename BVlT
        , template <class> class TVlT
        , class ... SuppInfoTs > class GenericDictionary;

template< typename BVlT
        , template <class> class TVlT
        , class ... SuppInfoTs > struct IndexingTraits {
    /// Template container type used to store entries indexed by name or index.
    template<typename KT, typename VT> using TIndex = std::unordered_map<KT, VT>;
    /// Template container type used to store supplementary information entries
    /// indexed by pointers of dictionary entries.
    template<typename KT, typename VT> using TPHash = std::unordered_map<KT, VT>;

    typedef GenericDictionary<std::string, BVlT, TVlT, SuppInfoTs...> Dictionary;
    typedef GenericDictionary<ListIndex, BVlT, TVlT, SuppInfoTs...> ListOfStructures;

    // One probably has to re-define this templated template class for
    // every SuppInfo set.
    //template<typename T> struct ParameterT : public SuppInfoTs ... {};
};

/// A dictionary entry representation. Keeps basic introspection info allowing
/// one quickly find out, whether the
template< typename BaseValueT
        , template <class> class TVlT
        , class ... SuppInfoTs >
class DictEntry {
public:
    typedef IndexingTraits<BaseValueT, TVlT, SuppInfoTs...> Traits;
private:
    union {
        typename Traits::Dictionary * toDict;
        typename Traits::ListOfStructures * toList;
        BaseValueT * toSngl;
    };
    enum : unsigned char {
        isSngl = 0x0,
        isList = 0x1,
        isDict = 0x2
    } code;
public:
    DictEntry( typename Traits::Dictionary * d ) : toDict( d ), code(isDict) {}
    DictEntry( typename Traits::ListOfStructures * l ) : toList( l ), code(isList) {}
    DictEntry( BaseValueT * v ) : toSngl( v ), code(isSngl) {}
    // ...
    //template<typename T> T * as_ptr();
};

template<typename T>
struct IndexingKeyTraits {
    typedef const T & KeyHandle;
    static inline std::string to_str( KeyHandle k ) {
        std::stringstream ss;
        ss << k;
        return ss.str();
    }
};

/**@brief Generic entry dictionary class.
 *
 * This template claims the basic logic for dealing with basic entries with
 * supplementary information composed from arbitrary set of types.
 **/
template< typename KeyT
        , typename BVlT
        , template <class> class TVlT
        , class ... SuppInfoTs >
class GenericDictionary : public mixins::iDuplicable< BVlT
                                                    , GenericDictionary<KeyT, BVlT, TVlT>
                                                    , TVlT< typename IndexingTraits<BVlT, TVlT, SuppInfoTs...> \
                                                                ::template TIndex<KeyT, DictEntry<BVlT, TVlT, SuppInfoTs...> > >
                                                    >
                        , public IndexingTraits<BVlT, TVlT, SuppInfoTs...> \
                                    ::template TPHash<DictEntry<BVlT, TVlT, SuppInfoTs...> *, SuppInfoTs>... {
public:
    typedef KeyT Key;
    typedef IndexingKeyTraits<KeyT> KeyTraits;
    typedef IndexingTraits<BVlT, TVlT, SuppInfoTs...> Traits;
    typedef GenericDictionary<KeyT, BVlT, TVlT, SuppInfoTs...> Self;
    typedef DictEntry<BVlT, TVlT, SuppInfoTs...> Entry;
    typedef typename Traits::template TIndex<KeyT, DictEntry<BVlT, TVlT, SuppInfoTs...> *> Index;
    typedef TVlT<Index> Parent;
private:
    template<typename SuppInfoT>
        typename std::enable_if<stdE::is_one_of<SuppInfoT, SuppInfoTs...>::value, bool >::type
    insert_info( Entry * e, SuppInfoT * i ) {
        auto ir = Traits::template TPHash<Entry *, SuppInfoT>::emplace( e, i );
        return ir.second;
    }
protected:
    /// Const variant of entry pointer retrieving function.
    const Entry * entry( typename KeyTraits::KeyHandle key ) const {
        auto it = Parent::value().find( key );
        if( Parent::value().end() == it ) {
            emraise( notFound, "Has no entry indexed by key \"%s\"."
                   , KeyTraits::to_str(key).c_str() );
        }
        return it->second;
    }

    /// Const version of supplementary information getter.
    template<typename SuppInfoT>
        typename std::enable_if<stdE::is_one_of<SuppInfoT, SuppInfoTs...>::value, const SuppInfoT * >::type
    info( typename KeyTraits::KeyHandle key ) const {
        auto ePtr = entry( key ) ;
        auto it = Traits::template TPHash<Entry *, SuppInfoT>::find( ePtr );
        return it->second;
    }

    /// Generic insertion method, dynamically supporting any set of subtypes.
    template<class ... EntrySuppInfoTs> void
    acquire_item( typename KeyTraits::KeyHandle key, Entry e, EntrySuppInfoTs ... ts ) {
        auto nameIR = Parent::_mutable_value().emplace( key, e );
        if( !nameIR.second ) {
            emraise( nonUniq, "Generic dictionary instance %p has entry with"
                    " index \"%s\": %p. Unable to insert parameter instance"
                    " referred by the same index."
                   , this, KeyTraits::to_str(key).c_str(), nameIR.first->second );
        }
        auto ePtr = &(*(nameIR.first));
        //bool eIRs[] = { (Traits::template TPHash<Entry *, EntrySuppInfoTs>::emplace( e, ts )) ... };
        bool eIRs[] = { nameIR.second, (insert_info(ePtr, ts)) ... };
        for( bool eIR : eIRs ) {
            if( !eIR ) {
                emraise( nonUniq, "Unable to associate supp info ? with"
                        " entry %p.", ePtr );
            }
        }
    }
};

# if 0
# include <map>
# include <set>

struct iBaseValue {
    // ...
};

struct DictEntry {
    enum EntryType {
        tcDict = 0x2,
        tcList = 0x1,
        tcSngl = 0x0,
    } typeCode;
    union {
        Dictionary<std::string> * dictPtr;
        Dictionary<ListIndex>   * listPtr;
        iBaseValue * snglPtr;
    } pointer;
};

template< typename KeyT
        , typename CustomPropsT >
class Dictionary : protected std::set<iBaseValue *>
                 , protected std::unordered_map<KeyT, DictEntry> {
    // ...
};

DictTraits<KeyT, CustomPropsT>::Dictcontainer
DictTraits<KeyT, CustomPropsT>::ListContainer
DictTraits<KeyT, CustomPropsT>::SingularsContainer

struct A {
    typedef std::map<std::string, iBaseValue *> Container;
};


int
main(int argc, char * argv[]) {

    Dictionary<A> a;

    return 0;
}

# endif

# if 0

/**@brief Type traits template defining general properties of parameter mapping.
 * @tparam KeyT Mapping key type.
 * @tparam ValueT Mapping value type.
 * @tparam hasNameFieldT Parameter used to infer specialized key-hashing logic.
 *
 * This traits abstraction defines some features raising on the indexing
 * containers within goo::dict framework. General case is pretty strightforward:
 * just defines usual key/value mapping.
 *
 * The special hasNameFieldT parameter is used to introduce template
 * specialization for AbstractParameter subclasses that keep their names within
 * value structure, to avoid unnecessary name duplication.
 * */
template< typename KeyT
        , typename ValueT
        , bool hasNameFieldT=std::is_base_of<AbstractParameter, ValueT>::value > struct IndexingTraits {
    /// Indexing key type.
    typedef KeyT Key;
    /// Type of value formally kept in container.
    typedef ValueT Value;
    /// Type of value physically kept in container.
    typedef Value * ValueHandle;
    /// Type of value physically kept in container (const variant).
    typedef const Value * ConstValueHandle;
    /// Type of container performing actual mapping.
    typedef Hash<Key, ValueHandle> IndexingContainer;
    /// Element iterator type.
    typedef typename IndexingContainer::iterator Iterator;
    /// Element iterator type (const variant).
    typedef typename IndexingContainer::const_iterator ConstIterator;
    /// Insertion result type. Must obey the usual STL map-insertion std::pair<>
    /// struct.
    typedef std::pair<Iterator, bool> InsertionResult;
    /// Iterator dereferencing action.
    static ValueHandle dereference_iterator( Iterator it )
        { return it->second; }
    /// Const iterator dereferencing action.
    static ConstValueHandle dereference_iterator( ConstIterator it )
        { return it->second; }
    /// Element look-up action.
    static Iterator find_item( const Key & k, IndexingContainer & c ) {
        return c.find( k );
    }
    /// Element look-up action (const variant).
    static ConstIterator find_item( const Key & k, const IndexingContainer & c ) {
        return c.find( k );
    }
    /// Element insertion action.
    static InsertionResult insert_item( const Key & k, ValueHandle v, IndexingContainer & c) {
        return c.emplace( k, v );
    }
};

/// Specialization for a special case: traits of container maintaining lifetime
/// of allocated parameter instances.
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

/**@class iDictionaryIndex
 *
 * This class implements insertion and retrieval procedures for mapping
 * containers. Except for special case of
 * iDictionaryIndex<void, iBaseValue, false>, it does not physically delete
 * values kept.
 * */
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

# endif

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_PLURAL_H

