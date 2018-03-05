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

# include "goo_dict/types.hpp"
# include "goo_dict/value.hpp"
# include "goo_mixins/vcopy.tcc"

# include <sstream>

namespace goo {
namespace dict {

template< typename KeyT
        , typename ... AspectTs>
class GenericDictionary;  // fwd

template< typename KeyT
        , template <class, class...> class ParameterT
        , class ... AspectTs >
class BaseInsertionProxy;  // fwd

template< typename KeyT
        , class ... AspectTs > struct InsertionAttorney;

/**@brief Common traits (pretty basic ones)
 *
 * The goo::dict traits are defined for every unique set of simple structures
 * called "aspects". By the language limitation, we can not define the set of
 * aspects as an actual "set" (i.e. unordered).
 * */
template< typename ... AspectTs >
struct Traits {
    /// Type of general value kept by dictionaries with the fixed aspects set.
    typedef iAbstractValue VBase;
    typedef iBaseValue<AspectTs...> FeaturedBase;
    /// The index (dictionary) traits may be re-defined within partial
    /// specialization for certain key type (whithin every unique aspects set).
    template<typename KeyT> struct IndexBy {
        // One probably will want to append the dictionary traits (within partial
        // specialization). By default, each dictionary aspect keeps a map
        // indexing a set of self-typed instance pointers by same key.
        /// Additional behaviour may be mixed in the GenericDictionary<> by the mean
        /// of partial specialization of this type. By default its an empty struct.
        struct Aspect { /* nothing special by default */ };
        //typedef TValue< Hash<KeyT, GenericDictionary<KeyT, AspectTs...>* > > Aspect;
        //        // ... public Value< Map< otherKeyT , Dictionary<KeyT, AspectTs...>* > >

        typedef GenericDictionary<KeyT, AspectTs...> Dictionary;

        /// Key-value pairs container, the mandatory part of any generic dictionary.
        /// One probably would desire to re-define the second AspectTs... pack in order
        /// to restrict the aspects set included at the dictionary within particular
        /// traits.
        typedef TValue< Hash<KeyT, FeaturedBase * >
                      , AspectTs ... // < this pack
                      > DictValue;

        /// In-place entry copying procedure. Takes the iterator to an existing
        /// mutable entry and pointer to owning dictionary. Copies the entry using
        /// dictionaries allocator overwriting the object the given iterator points
        /// to.
        static void copy_dict_entry( typename DictValue::Value::iterator it
                                   , Dictionary * /*dct*/ ) {
            // TODO: use allocator submitted by dict instance here
            //Dictionary::template BaseInsertionProxy<InsertableParameter> ip( *dct );
            it->second = goo::clone_as< iAbstractValue
                                      , FeaturedBase
                                      , FeaturedBase >( it->second );
        }

        template<typename CallableT>
        static CallableT each_entry_recursively_revise( CallableT c, Dictionary & d) {
            d.each_entry_revise( c );
            // ... in traits, there are the subsections iterating procedures,
            // depending on their particular assembly.
            return c;
        }

        template<typename CallableT>
        static CallableT each_entry_recursively_read( CallableT c, const Dictionary & d) {
            d.each_entry_read( c );
            // ... in traits, there are the subsections iterating procedures,
            // depending on their particular assembly.
            return c;
        }
    };
};

template<typename KeyT>
struct KeyTraits {
    static std::string to_str( const KeyT & k ) {
        std::ostringstream os;
        os << k;
        return os.str();
    }
};


/**@class GenericDictionary
 * @brief The GenericDictionary template class defines a relations between
 *        parameters sets indexed by some key values.
 *
 * The dictionary instance itself, basically, holds the key-value mapping where
 * for certain key corresponds a single parameter instance. It is possible,
 * however to make the dictionary index other dictionaries as well (via the
 * template specializations within Traits struct). In this case more complex
 * structure shall be introduced via the IndexOf<KeyT>::Aspect mixin.
 *
 * The GenericDictionary also "owns" the physical memory of the created entries
 * allowing the virtual (copying) ctor to perform its operations safely.
 */
template< typename KeyT
        , typename ... AspectTs>
class GenericDictionary : public DictionaryAllocator<AspectTs...>
                        , public mixins::iDuplicable< iAbstractValue /*typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue::Base*/
                                                    , GenericDictionary<KeyT, AspectTs ...>
                                                    , typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue
                                                    , DictionaryAllocator<AspectTs...> & >
                        , public Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue
                        , public Traits<AspectTs...>::template IndexBy<KeyT>::Aspect {
public:
    typedef DictionaryAllocator<AspectTs...> OwnAllocator;
    typedef Traits<AspectTs...> OwnTraits;
    typedef typename OwnTraits::template IndexBy<KeyT>::DictValue OwnDictValue;
    typedef typename OwnDictValue::Base OwnBaseValue;
    typedef mixins::iDuplicable< /*OwnBaseValue*/ iAbstractValue
                               , GenericDictionary<KeyT, AspectTs ...>
                               , OwnDictValue
                               , OwnAllocator &> DuplicableParent;
    /// The insertion proxy is a class to which the insertion permission is
    /// granted. It has to become a base class for particular insertion proxies
    /// implementing various entry-construction interfaces.
    typedef GenericDictionary<KeyT, AspectTs...> Self;
    typedef typename OwnTraits::template IndexBy<KeyT>::Aspect OwnAspect;

    template<template <class, class...> class ParameterT> struct BaseInsertionProxy {
        friend void Traits<AspectTs...>::template IndexBy<KeyT>::copy_dict_entry(
                        typename OwnTraits::template IndexBy<KeyT>::DictValue::Value::iterator it
                      , Self * dct );
    public:
        template<typename T> using P = ParameterT<T, AspectTs...>;
    private:
        Self & _target;
    protected:
        Self & target() { return _target; }
        /// Protected entry-insertion method.
        virtual std::pair<typename Hash<KeyT, iBaseValue<AspectTs...> *>::iterator, bool>
        _insert_parameter( const KeyT & k, typename OwnTraits::FeaturedBase * p ) {
            return target()._mutable_value().emplace(k, p);
        }
    protected:
        BaseInsertionProxy( Self & t ) : _target(t) {}

        /// This allocator is usually used by descendant classes to allocate
        /// aspects or some additional data which lifetime is bound with
        /// dictionary.
        template<typename T, typename ... CtrArgTs> T * _alloc( CtrArgTs ... args  ) {
            return _target.template _alloc<T>(args...);
        };
        /// A special method to allocate and initialize a new parameter instance.
        template<typename T> P<T> * _alloc_parameter( AspectTs * ... args ) {
            return _target.template _alloc< P<T> >(args...);
        };
    };
    //typedef mixins::iDuplicable< typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue::Base
    //                                                , GenericDictionary<KeyT, AspectTs ...>
    //                                                , typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue
    //                                                > DuplicableParent;
protected:
    // Protected entry-insertion method.
    //virtual std::pair<typename Hash<KeyT, iBaseValue<AspectTs...> *>::iterator, bool>
    //_insert_parameter( const KeyT & k, iBaseValue<AspectTs...> * p ) {
    //    return this->_mutable_value().emplace(k, p);
    //}
    //virtual typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue::Base *
    //                        _V_clone( AbstractValueAllocator & ) const override {
    //    _TODO_   // TODO
    //}
public:
    /// Common ctr. Copies the given allocator instance to self, sets own
    /// aspects with given aspects tuple, forwards the rest arguments to the
    /// aspect ctr as is.
    template<typename TT, typename ... CtrArgTs>
    GenericDictionary( OwnAllocator & da
                     , TT ownAspectsInitializer
                     , CtrArgTs ... ctrArgs ) \
            : OwnAllocator(da)
            , Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue( ownAspectsInitializer )
            , OwnAspect( ctrArgs ... ) {}

    # if 0
    GenericDictionary( const Self & orig ) = delete;
    # else
    GenericDictionary( const Self & orig
                     , OwnAllocator & da ) : OwnAllocator(da)
                                           , DuplicableParent( orig, da )
                                           , OwnAspect( static_cast<const OwnAspect &>(orig)
                                                      , static_cast<const OwnAspect *>(this)
                                                      , da ) {
        // All the contained entries reference the original's memory at this
        // point. We have to produce own copies here.
        // DuplicableParent's parent class refers to 'DictValue' type that
        // is defined by traits particular type.
        for( auto it = this->_mutable_value().begin()
           ; this->_mutable_value().end() != it; ++it ) {
            OwnTraits::template IndexBy<KeyT>::copy_dict_entry( it, this );
        }
    }
    # endif

    virtual ~GenericDictionary() {
        for( auto p : this->_mutable_value() ) {
            DictionaryAllocator<AspectTs...>::_free( p.second );
        }
    }

    /// Aux method purging entries without actual deletion of the value
    /// instances. Used to prevent the destructor of wiping out the shared
    /// entries.
    void drop_entries() {
        this->_mutable_value().clear();
    }

    // First level getters --- provides basic access to entries.

    /// Returns pointer to value or nullptr (if entry not found).
    virtual const typename OwnTraits::FeaturedBase * entry_ptr( const KeyT & k ) const {
        auto it = this->value().find(k);
        if( this->value().end() == it ) {
            return nullptr;
        }
        return (it->second);
    }

    /// Returns pointer to value or nullptr (if entry not found).
    virtual typename OwnTraits::FeaturedBase * entry_ptr( const KeyT & k ) {
        auto it = this->_mutable_value().find(k);
        if( this->_mutable_value().end() == it ) {
            return nullptr;
        }
        return (it->second);
    }

    // Second level getters.

    /// Returns mutable reference to value, or raises goo::notFound if not
    /// found.
    virtual typename OwnTraits::FeaturedBase & entry( const KeyT & k ) {
        auto ptr = entry_ptr(k);
        if( ! ptr ) {
            emraise( notFound
                   , "Dictionary %p has no entry with key \"%s\"."
                   , this
                   , KeyTraits<KeyT>::to_str(k).c_str() );
        }
        return *ptr;
    }

    /// Returns const reference to value, or raises goo::notFound if not
    /// found.
    virtual const typename OwnTraits::FeaturedBase & entry( const KeyT & k ) const {
        auto ptr = entry_ptr(k);
        if( ! ptr ) {
            emraise( notFound
                   , "Dictionary %p has no entry with key \"%s\"."
                   , this
                   , KeyTraits<KeyT>::to_str(k).c_str() );
        }
        return *ptr;
    }

    /// Template function performing iterative invocation of callable for each
    /// entry in current dictionary (mutable). The callable signature must
    /// accept value type of underlying container (map, or whatever is defined
    /// by traits).
    template<typename CallableT>
    CallableT each_entry_revise( CallableT c ) {
        for( auto it = this->_mutable_value().begin()
               ; it != this->_mutable_value().end(); ++it ) {
            c(it);
        }
        return c;
    }

    /// Template function performing iterative invocation of callable for each
    /// entry in current dictionary (const). The callable signature must
    /// accept value type of underlying container (map, or whatever is defined
    /// by traits).
    template<typename CallableT>
    CallableT each_entry_read( CallableT c ) const {
        for( auto it = this->value().begin()
               ; it != this->value().end(); ++it ) {
            c(it);
        }
        return c;
    }
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_PLURAL_H

