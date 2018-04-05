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

/**@brief Abstract base for instances within goo::dict.
 * @class iReferable
 *
 * This base expresses the most basic idea behind every entity within the
 * goo::dict framework by declaring a class that refers to physical data block.
 * No assumptions about the particular data structure or type are made here.
 *
 * C/C++ pointers returned by data_addr() family of functions and the
 * target_type_info() are in the sense of native C/C++ pointers and RTTI. The
 * rest may be customized.
 * */
class iReferable {
protected:
    /// Shall return untyped data pointer.
    virtual void * _V_data_addr() = 0;
    /// Shall return untyped (const) data pointer.
    virtual const void * _V_data_addr() const = 0;
    /// Shall return C++ RTTI type info.
    virtual const std::type_info & _V_target_type_info() const = 0;  // return typeid()
public:
    iReferable() = default;
    /// Returns untyped data pointer.
    void * data_addr() { return _V_data_addr(); }
    /// Returns untyped (const) data pointer.
    const void * data_addr() const { return _V_data_addr(); }
    /// Returns C++ RTTI value type info.
    virtual const std::type_info & target_type_info() const
                { return _V_target_type_info(); }
};  // class iEnumerable

namespace generic {

/**@brief Conversion rule, generic implementation.
 *
 * This generic implementation steers default conversion rules for target type
 * based on ordinary C++ conversion mechanic. To allow ser to define more
 * complex conversions, this class must be specialized.
 *
 * \tparam RealT type of value kept by wrapper (see ReferableWrapper)
 * \tparam DesiredT target conversion type */
template<template<typename> class X
        , typename RealT
        , typename DesiredT>
struct Dealer {
    static constexpr bool able =
            std::is_convertible< X<RealT>&, DesiredT>::value;
    typedef typename std::conditional<able, DesiredT, void>::type
            actual_ret_type;

    static actual_ret_type obtain(X <RealT> &t) { return t; }
    static const actual_ret_type obtain(const X <RealT> &t) { return t; }
};  // Dealer

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
        , typename ValueT
        , typename BaseT >
struct ReferableWrapper : public BaseT {  // NOTE: code below only makes sense with iReferable iface
private:
    X<ValueT> _v;
protected:
    virtual void * _V_data_addr() override { return &_v; }
    virtual const void * _V_data_addr() const override { return &_v; }
    virtual const std::type_info & _V_target_type_info() const override
                                        { return typeid(_v); }
public:
    template<typename ... ArgTs> ReferableWrapper( ArgTs ... ctrArgs )
                                                    : _v(ctrArgs...) {}
    template<typename DesiredT>
    typename std::enable_if< ReferableTraits<X>
                        ::template Dealer<ValueT, DesiredT>::able
                , typename ReferableTraits<X>
                        ::template Dealer<ValueT, DesiredT>::actual_ret_type
                >::type
        as() {
            return ReferableTraits<X>::template Dealer<ValueT, DesiredT>
                    ::obtain(_v); }

    template<typename DesiredT>
    typename std::enable_if< ReferableTraits<X>
    ::template Dealer<ValueT, DesiredT>::able
            , typename ReferableTraits<X>
            ::template Dealer<ValueT, DesiredT>::actual_ret_type
                           >::type
    as() const {
        return ReferableTraits<X>::template Dealer<ValueT, DesiredT>
                ::obtain(_v); }
    /// Directly returns contained instance.
    X<ValueT> & as_self() { return _v; }
    /// Directly returns contained instance (const).
    const X<ValueT> & as_self() const { return _v; }
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

    /**@brief Interim object returned by index operation.
     *
     * Aux object class suitable for further downcasting operations in
     * various lexical contexts: indexing (as a dict), retrieving values, etc.:
     * When operator[]() applied, tries to downcast the current entry to the
     * dictionary class defined by given key type, and with same X<> and
     * allocator template classes as in the containing (parent) dictionary.
     *
     * Be aware to not mex the allocator classes as downcasting is performed in
     * assumption that child dictionary uses the same allocator template class.
     * */
    template<typename ValueTypeT=typename Container::value_type>
    struct AccessProxy /* : public  */ {
        /// Usually refers to plain iterator of current map.
        ValueTypeT v;
        AccessProxy() = delete;
        explicit AccessProxy( ValueTypeT v_ ) : v(v_) {}
        /// This operator will try to consider subject as the dictionary indexed
        /// with keys of certain type.
        template<typename NextKeyT> auto operator[]( const NextKeyT & k ) {
            typedef typename ReferableTraits<X>::template DictionaryMessenger<NextKeyT> Subsection;
            auto sub = this->of<Subsection>();
            if( !sub ) {
                emraise( badParameter, "Unable to cast referable object %p [%s] "
                                       "to dictionary instance indexing "
                                       "with key [%s]."
                         , (void *) v.second
                         , std::to_string(v.first).c_str()
                         , std::to_string(k).c_str() );
            }
            return sub->template as<Subsection&>().operator[](k);
        }
        template<typename T> auto of() {
            return ReferableTraits<X>::template specify<T>( v.second );
        }
    };
public:
    /// Ctr forwarding args to Map parent class (value messenger).
    template<typename ... ArgTs> DictionaryWrapper( ArgTs ... ctrArgs )
            : Parent(ctrArgs...) {}

    Container & container() { return this->template as<Container&>(); }

    const Container & container() const { return this->template as<const Container&>(); }

    /// Returns pointer to referable instance of certain type or nullptr if not
    /// found. Internally, uses ReferableTraits<X>::specify<>() to perform a
    /// downcast, so may return nullptr in the case of bad cast either.
    template<typename T>
    typename Allocator<typename ReferableTraits<X>::template ReferableMessenger<T> >::pointer
    get_entry( const KeyT & k ) {
        //auto fr = Traits::Map<AllocatorT>::find( k );
        auto fr = Traits::find( this->container(), k );
        if( Traits::end( this->container() ) == fr ) {
            return nullptr;
        }
        return ReferableTraits<X>::template specify<T>( fr->second );
    }

    /// Returns pointer to typed referable instance or nullptr if not found
    /// (const form).
    template<typename T>
    typename Allocator<typename ReferableTraits<X>::template ReferableMessenger<T> >::const_pointer
    get_entry( const KeyT & k ) const {
        auto fr = Traits::find( this->container(), k );
        if( Traits::end( this->container() ) == fr ) {
            return nullptr;
        }
        return ReferableTraits<X>::template specify<T>( fr->second );
    }

    /// Performs in-place construction of an entry (as std::map::emplace()
    /// does), returning a pair of iterator and boolean value (with usual
    /// semantics).
    template<typename T, typename ... CtrArgTs>
    std::pair<typename Container::iterator, bool>
                    add_entry( const KeyT & k, CtrArgTs & ... ctrArgs ) {
        typedef typename ReferableTraits<X>
        ::template ReferableMessenger<T> TargetWrapper;
        auto it = Traits::find( this->container(), k );
        if( Traits::end( this->container() ) != it ) {
            // duplicate found, abort insertion, removing iterator constness:
            return std::pair<typename Container::iterator, bool>( container().erase(it, it), false );
        }
        // insertion of the new entry:
        auto cA = Traits::get_allocator( this->container() );
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
        return Traits::emplace( this->container(), k, newEMem );
    };

    /// Dictionary indexing operator. Returns an AccessProxy object referencing
    /// particular dictionary entry, or throws a goo::notFound exception if no
    /// entry found by given key.
    AccessProxy<typename Traits::reference> operator[]( const KeyT & k ) {
        auto it = Traits::find(this->container(), k);
        if( Traits::end( this->container() ) == it ) {
            emraise( notFound, "No entry by key [%s] exists in %p."
                     , std::to_string(k).c_str()
                     , this );
        }
        return AccessProxy<typename Traits::reference>(*it);
    }

    /// Dictionary indexing operator (const version).
    AccessProxy<typename Traits::value_type> operator[]( const KeyT & k ) const {
        auto it = Traits::find(this->container(), k);
        if( Traits::end( this->container() ) == it ) {
            emraise( notFound, "No entry by key [%s] exists in %p (const)."
                     , std::to_string(k).c_str()
                     , this );
        }
        return AccessProxy<typename Traits::value_type>(*it);
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
    /// Base type for referable containers. Usually untyped base of
    /// ReferableWrapper, to be indexed within the dictionaries. This type is
    /// supposed to be actually stored by dictionaries' mapping container and
    /// will be the 'second' part of value_type pair tuple. Conversion from
    /// this generic referable messenger type to the particular messenger type
    /// has to be performed by ::specify<>() static method.
    typedef iReferable ReferableMessengerBase;
    /// Messenger is a wrapper around particular X<T> defining common data
    /// access and modification operations.
    template <typename ValueT> using ReferableMessenger
                = generic::ReferableWrapper<X, ValueT, ReferableMessengerBase>;
    /// The "dealer" template defines how the value of particular type may be
    /// obtained from particular messenger. Its functions are typically
    /// exploited by messenger template class(es).
    template <typename RealT, typename DesiredT> using Dealer = generic::Dealer<X, RealT, DesiredT>;
    /// Allocator template defined for the X.
    template<typename T> using Allocator = std::allocator<T>;
    /// Defines type downcast conversion: from AbstractReferable * to X<T> *.
    /// May rely on various mechanisms -- from C++ RTTI to user-defined
    /// allocator-based instance indexing logic (depending on ReferableTraits
    /// specialization).
    template< typename T >
    static typename Allocator<ReferableMessenger<T> >::pointer specify(
                        typename Allocator<ReferableMessengerBase>::pointer arf ) {
        return dynamic_cast<typename Allocator<ReferableMessenger<T> >::pointer >( arf );
    }
    /// Defines type downcast conversion: from const AbstractReferable * to const X<T> *.
    template< typename T >
    static const typename Allocator<ReferableMessenger<T> >::const_pointer specify(
                    typename Allocator<ReferableMessengerBase>::const_pointer arf ) {
        return dynamic_cast<typename Allocator<ReferableMessenger<T> >::const_pointer >( arf );
    }

    /// Physical container type.
    template< typename KeyT>
    using MappingContainer = typename generic::ContainerTraits<KeyT, X, Allocator>::Container;

    /// Referable type keeping the dictionary within.
    template< typename KeyT> using Dictionary = X<MappingContainer<KeyT> >;

    /// Messenger referencing the dictionary.
    template<typename KeyT> using DictionaryMessenger = generic::DictionaryWrapper<X, KeyT>;
};

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_VALUES_H
