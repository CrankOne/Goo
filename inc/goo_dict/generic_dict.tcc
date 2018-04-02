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

# ifndef H_GOO_PARAMETERS_GENERIC_DICT_H
# define H_GOO_PARAMETERS_GENERIC_DICT_H

# include "goo_dict/types.hpp"
# include "goo_dict/value.hpp"
# include "goo_mixins/vcopy.tcc"

# include <sstream>
# include <unordered_map>
# include <stack>

# include <iostream>  // xxx

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

namespace goo {
namespace dict {

template< typename KeyT
        , template <typename> class X
        >
struct IndexingTraits {
    /**@brief Container type.
     *
     * Shall define interface similar to the STL map container: (const) iterator,
     * types, insert/emplace/erase/find methods returning results of standard
     * (defined by standard) types.
     * */
    template< template <typename> class AllocatorT > using Map
            = std::unordered_map< KeyT
                                , typename AllocatorT<typename ReferableTraits<X>::AbstractReferable>::pointer
                                , std::hash<KeyT>
                                , std::equal_to<KeyT>
                                , AllocatorT< std::pair<const KeyT, typename AllocatorT<typename ReferableTraits<X>::AbstractReferable>::pointer> > >;
    /// Converts key value to human-readable form, in a string expression.
    static std::string key_to_str( const KeyT & k ) {
        std::ostringstream ss;
        ss << k;
        return ss.str();
    }
};

/**@brief Dictionary is an object defining mapping between keys and referables.
 *
 * This class defines few helper functions shortening the routine
 * insertion/retrieval operations provided by underlying
 * IndexingTraits<KeyT, X>::Map class.
 *
 * Besides of methods inherited from mapping container, provides:
 *  - get_ptr() method performing search for certain entry and downcast'ing it;
 *    Note, that this method may return null pointer if entry is not found by
 *    the given key AND if down cast fails.
 *
 * @tparam KeyT Key type indexing the references.
 * @tparam X Template referable class.
 * @tparam Map Template container performing actual mapping.
 */
template< typename KeyT
        , template <typename> class X
        , template <typename> class AllocatorT=std::allocator >
class Dictionary : protected IndexingTraits<KeyT, X>::template Map< AllocatorT > {
public:
    /// Exposing template parameter: key type.
    typedef KeyT Key;
    /// Exposing template parameter: referable template class.
    template<typename T> using Referable = X<T>;
    /// Exposing template parameter: allocator template.
    template<typename T> using Allocator = AllocatorT<T>;
    /// Particular traits for external usage.
    typedef IndexingTraits<Key, X> Traits;
    /// Particular IndexingTraits<KeyT, X>::Map ancestor type
    typedef typename IndexingTraits<KeyT, X>::template Map< AllocatorT > TheMap;
    /// Returns interim object, suitable for further downcasting operations in
    /// various contexts: indexing (as a dict), retrieving values, etc.
    template<typename ValueTypeT=typename TheMap::value_type&>
    struct AccessProxy {
        ValueTypeT v;
        AccessProxy() = delete;
        AccessProxy( ValueTypeT v_ ) : v(v_) {}
        /// This operator will try to consider subject as the dictionary indexed
        /// with keys of certain type.
        template<typename NextKeyT> auto operator[]( const NextKeyT & k ) {
            typedef Dictionary<NextKeyT, X, AllocatorT> Subsection;
            auto sub = this->of<Subsection>();
            if( !sub ) {
                emraise( badParameter, "Unable to cast referable object %p [%s] "
                                       "to dictionary instance indexing "
                                       "with key [%s]."
                                     , (void *) v.second
                                     , IndexingTraits<KeyT, X>::key_to_str(v.first).c_str()
                                     , IndexingTraits<NextKeyT, X>::key_to_str(k).c_str() );
            }
            return sub->template as<Subsection&>().operator[](k);
        }
        template<typename T> auto of() {
            return ReferableTraits<X>::template specify<T, AllocatorT>( v.second );
        }
    };
public:
    using TheMap::begin;
    using TheMap::end;
    using TheMap::find;
    using TheMap::emplace;
    using TheMap::insert;

    /// Ctr forwarding args to Map class defined by IndexingTraits<KeyT, X>::Map.
    template<typename ... CtrArgTs>
    Dictionary(CtrArgTs && ... ctrArgs) : TheMap( ctrArgs ... ) {}

    /// Returns pointer to typed referable instance or nullptr if not found.
    template<typename T>
    typename AllocatorT<typename ReferableTraits<X>::template ReferableWrapper<T> >::pointer
    get_ptr( const KeyT & k ) {
        auto fr = this->find( k );
        if( this->end() == fr ) {
            return nullptr;
        }
        return ReferableTraits<X>::template specify<T, AllocatorT>( fr->second );
    }

    /// Returns pointer to typed referable instance or nullptr if not found (const).
    template<typename T>
    typename AllocatorT<typename ReferableTraits<X>::template ReferableWrapper<T> >::const_pointer
    get_ptr( const KeyT & k ) const {
        auto fr = this->find( k );
        if( this->end() == fr ) {
            return nullptr;
        }
        return ReferableTraits<X>::template specify<T, AllocatorT>( fr->second );
    }

    template<typename T, typename ... CtrArgTs>
    auto situate( const KeyT & k, CtrArgTs ... ctrArgs ) {
        typedef typename ReferableTraits<X>
              ::template ReferableWrapper<T> TargetWrapper;
        auto it = this->find( k );
        if( this->end() != it ) {
            // duplicate found, abort insertion
            return std::pair<typename TheMap::iterator, bool>( it, false );
        }
        // insertion of the new entry:
        auto cA = this->get_allocator();
        // ^^^ orig alloc instance (alloc'ing pairs)
        typedef typename std::allocator_traits<decltype(cA)>
                       ::template rebind_traits<TargetWrapper> SubATraits;
        typename std::allocator_traits<decltype(cA)>
               ::template rebind_alloc<TargetWrapper> subA(cA);
        // ^^^ new alloc (alloc'ing wrappers)
        // alloc & construct new entry:
        auto newEMem = SubATraits::allocate( subA, 1 );
        SubATraits::construct( subA, newEMem, ctrArgs... );
        // insert new entry
        return this->emplace( k, newEMem );
    };

    AccessProxy<typename TheMap::reference> operator[]( const KeyT & k ) {
        auto it = TheMap::find(k);
        if( TheMap::end() == it ) {
            emraise( notFound, "No entry by key [%s] exists in %p."
                             , IndexingTraits<KeyT, X>::key_to_str(k).c_str()
                             , this );
        }
        return *it;
    }

    AccessProxy<typename TheMap::const_reference> operator[]( const KeyT & k ) const {
        auto it = TheMap::find(k);
        if( TheMap::end() == it ) {
            emraise( notFound, "No entry by key [%s] exists in %p (const)."
                     , IndexingTraits<KeyT, X>::key_to_str(k).c_str()
                     , this );
        }
        return *it;
    }
};  // class Dictionary


/**@brief A helper class providing advanced syntax for dict hierarchy.
 *
 * The insertion proxies become convenient tool for in-place construction
 * of complex dictionaries. They're keeping the "insertion stack" which
 * top refers to current insertion target and shall be updated as new
 * sub-dictionaries ("sections") are added or finalized.
 *
 * Insertion proxy objects exist only in current process (thus may use ordinary
 * pointers) and are generally slow. Their primary purpose is to shorten some
 * common initialization procedures.
 */
template<typename DictionaryT>
class InsertionProxyBase {
public:
    /// Stack type of sections folded one into another. Ordinary on-heap
    /// allocating STL stack container (though its entries may be not
    /// allocated on heap).
    typedef std::stack< std::pair< const std::type_info *
                                 , typename DictionaryT::TheMap::value_type
                                 >
                      > Stack;
protected:
    /// Stack of sections, folded one into another. Stack top is the
    /// current insertion target.
    Stack _stack;
public:
    template< typename NewKeyT
            , typename ... CtrArgTs> auto new_section( const NewKeyT & key
                                          , CtrArgTs && ... ctrArgs ) {
        auto ir = _stack.top().emplace( key, ctrArgs ... );
        if( !ir.second ) {
            emraise( nonUniq, "Insertion proxy object %p unable to add new"
                   " section within dictionary instance %p since key \"%s\""
                   " is not unique."
                   , this
                   , &_stack.top()
                   , DictionaryT::Traits::key_to_str(key).c_str() );
        }
        _stack.push( std::make_pair( &typeid(NewKeyT), ir.first.second ) );
        return ir;
    }
    template< typename NewKeyT > void close_section( const NewKeyT & key ) {
        // ...
    }
};

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_GENERIC_DICT_H

