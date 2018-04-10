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

// Created by crank on 12.01.18.

# ifndef H_GOO_PARAMETERS_VALUES_H
# define H_GOO_PARAMETERS_VALUES_H

# include "goo_dict/types.hpp"
# include "goo_mixins/vcopy.tcc"
# include "goo_utility.hpp"

# include "goo_dict/asp_unwind.tcc"

# include <tuple>
# include <sstream>
# include <cassert>

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

namespace goo {
namespace dict {

template< template<typename> class X > struct ReferableTraits;

// todo: move it outta here or use boost::call_traits or whatsoever
template<typename T>
struct CallTraits {
    typedef T & MutableRef;
    typedef const T & ConstRef;
};

/// Reflexive traits defined for certain introspection info type.
template< typename TypeID
        , typename T> struct AReflexiveTraits;

template<typename T>
struct AReflexiveTraits<const std::type_info &, T> {
    static const std::type_info & get_type_id() { return typeid(T); }
};

namespace generic {

/**@brief Abstract base for instances within goo::dict.
 * @class iReferable
 *
 * This base expresses the most basic idea behind every entity within the
 * goo::dict framework by declaring a class that refers to physical data block.
 * No assumptions about the particular data structure or type are made here
 * except claiming the reflexiveness feature for subsequent classes.
 * */
template<typename TypeIDT>
class iReferable {
public:
    typedef TypeIDT TypeID;
private:
    /// Shall return C++ RTTI type info.
    virtual TypeID _V_target_type_id() const = 0;
public:
    iReferable() = default;
    /// Returns C++ RTTI value type info.
    virtual TypeID target_type_id() const {
        return _V_target_type_id();
    }
};  // class iEnumerable

/**@brief Referable entity, generic implementation.
 *
 * This generic implementation represents shimmering template class providing
 * access to various value conversions. It is a routing point to forward value
 * handling invocations to various dealer classes (which are merely the
 * strategy pattern). User may want to re-define this behaviour in some rare
 * cases too (do it via the template specialization).
 * @tparam X Value holder class template.
 * @tparam ValueT actual value type.
 * @tparam BaseT customizable base class for entity (used by traits). */
template< template<typename> class X
        , typename ValueT >
struct ReferableWrapper : public ReferableTraits<X>::ReferableMessengerBase {
private:
    X<ValueT> _v;
private:
    typename ReferableTraits<X>::TypeInfo _V_target_type_id() const final {
        return ReferableTraits<X>::template ReflexiveTraits<ValueT>::get_type_id();
    }
public:
    template<typename ... ArgTs> explicit ReferableWrapper( ArgTs ... ctrArgs ) : _v(ctrArgs...) {}
    /// Directly returns contained instance.
    X<ValueT> & unwrap() { return _v; }
    /// Directly returns contained instance (const).
    const X<ValueT> & unwrap() const { return _v; }
    /// Delegates conversion/extraction to traits to obtain value of particular
    /// type from wrapped X<T> (mutable ver).
    template<typename T> typename CallTraits<T>::MutableRef unwrap_as() {
        return _unwrap_as_impl( *this );
    }
    /// Delegates conversion/extraction to traits to obtain value of particular
    /// type from wrapped X<T> (const ver).
    template<typename T> typename CallTraits<T>::ConstRef unwrap_as() const {
        return _unwrap_as_impl( *this );
    }
private:
    template<typename T, typename ThisT> auto _unwrap_as_impl( ThisT & t ) -> decltype(t. template unwrap_as<T>()) {
        ReferableTraits<X>::template obtain<ValueT, T>( this->unwrap() );
    }
};  // Traits<X>::ReferableMessenger (generic)

/**@brief Traits scope defining the container properties.
 *
 * It is possible to implement various indexing models (e.g. trees, hash maps)
 * that establishes correspondence between two classes of objects. The
 * particular container may be physically implemented in various ways, but
 * default traits implementation defines std::unordered_map as most versatile
 * one.
 *
 * Most of the types have the usual STL sense (pointer, reference, value_type)
 * and their meaning is covered by STL documentation.
 * @tparam KeyT indexing key defining
 * @tparam X Value holder class template
 * @tparam AllocatorT allocator class template */
template< typename KeyT
        , template <typename> class X
        , template <typename> class AllocatorT>
struct ContainerTraits {
    /// Indexing key type.
    typedef KeyT Key;
    /// Value type kept by the hashing container.
    typedef typename AllocatorT<typename ReferableTraits<X>
    ::ReferableMessengerBase>::pointer Value;
    /**@brief Physical container instance type to deal with.
     *
     * Shall define interface similar to the STL map container: (const) iterator,
     * types, insert/emplace/erase/find methods returning results of standard
     * (defined by standard) types.
     * */
    typedef std::unordered_map< Key
            , Value
            , std::hash<Key>
            , std::equal_to<Key>
            , AllocatorT<std::pair<Key, Value> > > Container;
    typedef typename Container::value_type value_type;
    typedef typename Container::reference reference;
    typedef typename Container::reference const_reference;
    /// Plain iterator follows only the horizontal level of hierarchy.
    typedef typename Container::iterator PlainIterator;
    /// Plain iterator follows only the horizontal level of hierarchy (const).
    typedef typename Container::const_iterator ConstPlainIterator;
    /// Returns the first plain iterator value.
    static ConstPlainIterator begin( const Container & m ) {
        return m.begin();
    }
    /// Returns the end plain iterator value (after last one).
    static ConstPlainIterator end( const Container & m ) {
        return m.end();
    }
    /// Inserts new element into the container.
    static auto insert( Container & m, const KeyT & k, Value & v ) {
        return m.insert( k, v );
    }
    /// In-place constructions of new element.
    template<typename ... CtrArgTs>
    static auto emplace( Container & m, const KeyT & k, CtrArgTs & ... ctrArgs ) {
        return m.emplace( k, ctrArgs ... );
    }
    /// Returns plain iterator pointing to the element with certain key or
    /// end iterator if no entry found.
    static PlainIterator find( Container & m, const KeyT & k ) {
        return m.find( k );
    }
    /// Returns plain iterator pointing to the element with certain key or
    /// end iterator if no entry found (const version).
    static ConstPlainIterator find( const Container & m, const KeyT & k ) {
        return m.find( k );
    }
    /// Returns associated allocator instance (typically, for rebinding usage).
    static auto get_allocator( const Container & m ) {
        return m.get_allocator();
    }
};

/**@brief Key/value index template, generic implementation.
 *
 * Dictionary is an object defining mapping between keys and referable entity.
 *
 * This class defines few helper functions shortening the routine
 * insertion/retrieval operations provided by underlying container class, that
 * is defined by the ContainerTraits.
 *
 * Note that allocator type will be defined by ReferableTraits.
 *
 * Besides of methods inherited from usual value messenger container, provides:
 *  - get_ptr() method performing search for certain entry and downcast'ing it;
 *    Note, that this method may return null pointer if entry is not found by
 *    the given key AND if down cast fails.
 *
 * @tparam X Template referable class.
 * @tparam Map Template container performing actual mapping.
 */
template< template<typename> class X
        , typename KeyT>
struct DictionaryWrapper : public ReferableTraits<X>::template ReferableMessenger<
                                typename ReferableTraits<X>::template MappingContainer<KeyT>
                            > {
public:
    typedef typename ReferableTraits<X>::template MappingContainer<KeyT> Container;
    typedef typename ReferableTraits<X>::template ReferableMessenger<Container> Parent;
    /// Exposing template parameter: key type.
    typedef KeyT Key;
    /// Exposing template parameter: referable template class.
    template<typename T> using Referable = X<T>;
    /// Exposing template parameter: allocator template.
    template<typename T> using Allocator = typename ReferableTraits<X>::template Allocator<T>;
    /// Particular traits for external usage.
    typedef ContainerTraits<Key, X, Allocator> Traits;
public:
    /// Ctr forwarding args to Map parent class (value messenger).
    template<typename ... ArgTs> DictionaryWrapper( ArgTs ... ctrArgs )
            : Parent(ctrArgs...) {}

    /// Performs in-place construction of an entry (as std::map::emplace()
    /// does), returning a pair of iterator and boolean value (with usual
    /// semantics).
    /// TODO: move implem to insertion proxy class
    template<typename T, typename ... CtrArgTs>
    std::pair<typename Container::iterator, bool>
                    add_entry( const KeyT & k, CtrArgTs & ... ctrArgs ) {
        typedef typename ReferableTraits<X>
                    ::template ReferableMessenger<T> TargetWrapper;
        auto it = Traits::find( this->template unwrap_as<Container>(), k );
        if( Traits::end( this->template unwrap_as<Container>() ) != it ) {
            // duplicate found, abort insertion, removing iterator constness:
            return std::pair<typename Container::iterator, bool>( this->template unwrap_as<Container>().erase(it, it), false );
        }
        // insertion of the new entry:
        auto cA = Traits::get_allocator( this->template unwrap_as<Container>() );
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
        return Traits::emplace( this->template unwrap_as<Container>(), k, newEMem );
    };

    /// TODO: move implem to insertion proxy class
    template<typename SubKeyT, typename ... CtrArgTs>
    std::pair<typename Container::iterator, bool>
    add_subsect( const SubKeyT & k, CtrArgTs & ... ctrArgs ) {
        typedef typename ReferableTraits<X>
                ::template DictionaryMessenger<SubKeyT> TargetWrapper;
        auto it = Traits::find( this->template unwrap_as<Container>(), k );
        if( Traits::end( this->template unwrap_as<Container>() ) != it ) {
            // duplicate found, abort insertion, removing iterator constness:
            return std::pair<typename Container::iterator, bool>(
                    this->template unwrap_as<Container>().erase(it, it), false );
        }
        // insertion of the new entry:
        auto cA = Traits::get_allocator( this->template unwrap_as<Container>() );
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
        return Traits::emplace( this->template unwrap_as<Container>(), k, newEMem );
    };

    /// Returns pointer to referable instance of certain type or nullptr if not
    /// found. Internally, uses ReferableTraits<X>::specify<>() to perform a
    /// downcast, so may return nullptr in the case of bad cast either.
    template<typename T>
    typename Allocator<typename ReferableTraits<X>::template ReferableMessenger<T> >::pointer
    entry_ptr( const KeyT & k ) {
        return _get_entry_impl<KeyT, T>(k, *this);
    }

    /// Returns pointer to typed referable instance or nullptr if not found
    /// (const form).
    template<typename T>
    typename Allocator<typename ReferableTraits<X>::template ReferableMessenger<T> >::const_pointer
    entry_ptr( const KeyT & k ) const {
        return _get_entry_impl<T>(k, *this);
    }

    template<typename SubKeyT>
    typename Allocator<typename ReferableTraits<X>::template DictionaryMessenger<SubKeyT> >::pointer
    subsect_ptr( const SubKeyT & k ) {
        return _subsect_ptr_implem( k, *this );
    }

    template<typename SubKeyT>
    typename Allocator<typename ReferableTraits<X>::template DictionaryMessenger<SubKeyT> >::const_pointer
    subsect_ptr( const SubKeyT & k ) const {
        return _subsect_ptr_implem( k, *this );
    }
private:
    template< typename T, typename ThisT >
    static auto _get_entry_impl( const KeyT & k, ThisT & t ) -> decltype(t.get_entry(k)) {
        auto fr = Traits::find( t.template unwrap_as<Container>(), k );
        if( Traits::end( t.template unwrap_as<Container>() ) == fr ) {
            return nullptr;
        }
        return ReferableTraits<X>::template specify<T>( fr->second );
    }

    template< typename SubKeyT, typename ThisT >
    static auto _subsect_ptr_implem( const SubKeyT & k, ThisT & t ) -> decltype(t.subsect_ptr(k)) {
        auto ePtr = t.entry_ptr( k );
        return t.get_allocator().address(ePtr->as_index_by<SubKeyT>());
    }
};  // class DictionaryWrapper

}  // namespace generic

/**@brief Generic traits for goo::dict's entries.
 *
 * This traits struct defines the basic properties of hierarchical structures
 * built over a superset induced by a template class.
 *
 * @tparam X Value keeper template class.
 */
template< template<typename> class X >
struct ReferableTraits {
    /// Defines instance extraction rule.
    template< typename RealT, typename DesiredT >
    static typename CallTraits<DesiredT>::MutableRef obtain( X<RealT> & obj ) {
        return (DesiredT &) obj;
    };

    /// Defines instance extraction rule (const ref).
    template< typename RealT, typename DesiredT >
    static typename CallTraits<DesiredT>::ConstRef obtain( const X<RealT> & obj ) {
        return (const DesiredT &) obj;
    };

    typedef const std::type_info & TypeInfo;
    /// Alias to corresponding reflexive traits
    template<typename T> using ReflexiveTraits
            = AReflexiveTraits<TypeInfo, T>;
    /// Allocator template defined for the X.
    template<typename T> using Allocator = std::allocator<T>;

    /// Dictionary traits.
    template<typename KeyT> using DictionaryTraits = generic::ContainerTraits<KeyT, X, Allocator>;
    /// Physical container type.
    template< typename KeyT> using MappingContainer = typename DictionaryTraits<KeyT>::Container;
    /// Referable type keeping the dictionary within.
    template< typename KeyT> using Dictionary = X<MappingContainer<KeyT> >;
    /// Messenger referencing the dictionary.
    template<typename KeyT> using DictionaryMessenger = generic::DictionaryWrapper<X, KeyT>;

    /// Base type for referable containers. Usually untyped base of
    /// ReferableWrapper, to be indexed within the dictionaries. This type is
    /// supposed to be actually stored by dictionaries' mapping container and
    /// will be the 'second' part of value_type pair tuple. Conversion from
    /// this generic referable messenger type to the particular messenger type
    /// has to be performed by ::specify<>() static method.
    /// At this point, the as<>() magic may be involved, since here we know the
    /// X superset-defining template class, allocator info and whatsoever...
    ///@todo: move this implem to generic ns?
    class ReferableMessengerBase : public generic::iReferable<TypeInfo> {
    public:
        /// Returns a mutable reference to an instance of type X<T> performing
        /// safe downcast.
        template<typename T>
        typename CallTraits<T>::MutableRef as() {
            return _as_impl<T>(*this);
        }
        /// Returns a const reference to an instance of type X<T> performing
        /// safe downcast.
        template<typename T>
        typename CallTraits<T>::ConstRef   as() const {
            return _as_impl<T>(*this);
        }
        /// Returns mutable reference to a dictionary indexed by key of type
        /// X<T>.
        template<typename KeyT>
        DictionaryMessenger<KeyT> & as_index_by() {
            return _as_index_by_impl<KeyT>(*this);
        }
        /// Returns const reference to a dictionary indexed by key of type
        /// X<T>.
        template<typename KeyT>
        const DictionaryMessenger<KeyT> & as_index_by() const {
            return _as_index_by_impl<KeyT>(*this);
        }
        /// Tries to consider current entry as a dictionary indexed by key of
        /// type T and return the mutable entry referenced by given key.
        template<typename NextKeyT> ReferableMessengerBase & operator[]( const NextKeyT & k ) {
            return as_index_by<NextKeyT>()[k];
            // todo: append possible exception w/ string repr of key?
        }
        /// Tries to consider current entry as a dictionary indexed by key of
        /// type T and return the const entry referenced by given key.
        template<typename NextKeyT> const ReferableMessengerBase & operator[]( const NextKeyT & k ) const {
            return as_index_by<NextKeyT>()[k];
            // todo: append possible exception w/ string repr of key?
        }
    private:
        /// Template implementation for as<T>() for (non-)const versions.
        template<typename T, typename ThisT>
        static auto _as_impl( ThisT & t ) -> decltype(t.template as<T>()) {
            // todo: exact match prohibits conversion.
            if( ReflexiveTraits<T>::get_type_id() != t.target_type_id() ) {
                emraise( badCast, "Can not downcast entry %p to requested"
                                  " type.", &t );
            }
            return ReferableTraits::specify<T>(&t)->unwrap();
        }
        /// Template implementation for as_index_by<T>() for (non-)const
        /// versions.
        template<typename KeyT, typename ThisT>
        static auto _as_index_by_impl( ThisT & t ) -> decltype(t.template as_index_by<KeyT>()) {
            typedef MappingContainer<KeyT> DesiredDict;
            if( ReflexiveTraits<DesiredDict>::get_type_id() != t.target_type_id() ) {
                emraise( badCast, "Can not downcast entry %p to dictionary"
                                  " of requested type", &t );
            }
            auto & a = *(ReferableTraits::specify<DesiredDict>(&t));  // todo: allocator::dereference?
            return static_cast<decltype(t.template as_index_by<KeyT>())&>(a);
        }
    };
    /// Messenger is a wrapper around particular X<T> defining common data
    /// access and modification operations.
    template <typename ValueT> using ReferableMessenger
                = generic::ReferableWrapper<X, ValueT>;
    /// Defines type downcast conversion: from AbstractReferable * to X<T> *.
    /// May rely on various mechanisms -- from C++ RTTI to user-defined
    /// allocator-based instance indexing logic (depending on ReferableTraits
    /// specialization).
    template< typename T >
    static typename Allocator<ReferableMessenger<T> >::pointer specify(
                        typename Allocator<ReferableMessengerBase>::pointer arf ) {
        return _specify_impl<T>( arf );
    }
    /// Defines type downcast conversion: from const AbstractReferable * to const X<T> *.
    template< typename T >
    static const typename Allocator<ReferableMessenger<T> >::const_pointer specify(
                    typename Allocator<ReferableMessengerBase>::const_pointer arf ) {
        return _specify_impl<T>( arf );
    }
private:
    template<typename T, typename PtrT>
    static auto _specify_impl( PtrT arf ) -> decltype(specify<T>(arf)) {
        // TODO: rely on ReflexiveTraits's downcast introspection rather than on C++ RTTI.
        auto ptr = dynamic_cast<
                typename std::conditional<
                            std::is_same<PtrT, typename Allocator<ReferableMessengerBase>::pointer>::value
                          , typename Allocator<ReferableMessenger<T> >::pointer
                          , typename Allocator<ReferableMessenger<T> >::const_pointer
                          >::type
                >( arf );
        if( !ptr ) {
            emraise( badCast, "Unable to specify %p.", arf );
        }
        return ptr;
    }
};

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_VALUES_H
