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
 * aspects as an actual "set" (i.e. unordered)
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
        typedef TValue< Hash<KeyT, iBaseValue<AspectTs...>*>
                      , AspectTs ... // < this pack
                      > DictValue;
    };
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
 */
template< typename KeyT
        , typename ... AspectTs>
class GenericDictionary : public mixins::iDuplicable< typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue::Base
                                                    , GenericDictionary<KeyT, AspectTs ...>
                                                    , typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue
                                                    >
                        , public Traits<AspectTs...>::template IndexBy<KeyT>::Aspect {
public:
    typedef Traits<AspectTs...> OwnTraits;
    /// The insertion proxy is a class to which the insertion permission is
    /// granted. It has to become a base class for particular insertion proxies
    /// implementing various entry-construction interfaces.
    typedef GenericDictionary<KeyT, AspectTs...> Self;
    template<template <class, class...> class ParameterT> struct BaseInsertionProxy {
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
            return _target._alloc<T>(args...);
        };
        /// A special method to allocate and initialize a new parameter instance.
        template<typename T> P<T> * _alloc_parameter( AspectTs * ... args ) {
            return _target._alloc< P<T> >(args...);
        };
    };
    typedef mixins::iDuplicable< typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue::Base
                                                    , GenericDictionary<KeyT, AspectTs ...>
                                                    , typename Traits<AspectTs ...>::template IndexBy<KeyT>::DictValue
                                                    > DuplicableParent;
protected:
    /// Called by insertion proxies to allocate data of various types, including
    /// instances of particular parameter types.
    template<typename T, typename ... CtrArgTs> T * _alloc(CtrArgTs ... args) {
        // TODO: use more advanced allocation logic here.
        return new T(args...);
    }
    template<typename T> void _free( T * p ) {
        // TODO: use more advanced allocation logic here.
        delete p;
    }
    // Protected entry-insertion method.
    //virtual std::pair<typename Hash<KeyT, iBaseValue<AspectTs...> *>::iterator, bool>
    //_insert_parameter( const KeyT & k, iBaseValue<AspectTs...> * p ) {
    //    return this->_mutable_value().emplace(k, p);
    //}
public:
    /// Common ctr, that barely forwards arguments to aspect ctr.
    template<typename TT, typename ... CtrArgTs>
    GenericDictionary( TT ownAspectsInitializer
                     , CtrArgTs ... ctrArgs ) \
            : DuplicableParent( ownAspectsInitializer )
            , Traits<AspectTs...>::template IndexBy<KeyT>::Aspect( ctrArgs ... ) {}

    virtual ~GenericDictionary() {
        for( auto p : this->_mutable_value() ) {
            //_free( p.second );  // TODO !!!
        }
    }

    /// Returns pointer to entry or nullptr (if entry not found).
    template<typename T> T * retrieve( const KeyT & k ) {
        auto it = this->_mutable_value().find(k);
        if( this->_mutable_value() == it ) {
            return nullptr;
        }
        return static_cast<T *>(*(it->second));
    }
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_PLURAL_H

