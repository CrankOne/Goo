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

// Common traits (pretty basic ones)
template< typename ... AspectTs >
struct Traits {
    /// Type of general value kept by dictionaries with the fixed aspects set.
    typedef iBaseValue<AspectTs ...> VBase;
    // One probably will want to append the dictionary traits (within partial
    // specialization). By default, each dictionary aspect keeps a map
    // indexing a set of self-typed instance pointers by same key.
    /// Additional behaviour may be mixed in the GenericDictionary<> by the mean
    /// of partial specialization of this type. By default its an empty struct.
    template<typename KeyT> struct DictionaryAspect :
            public TValue< Hash<KeyT, GenericDictionary<KeyT, AspectTs...>* > >
            // ... public Value< Map< otherKeyT , Dictionary<KeyT, AspectTs...>* > >
    { };

    template<typename KeyT> using Dictionary = GenericDictionary<KeyT, AspectTs...>;
};

template< typename KeyT
        , typename ... AspectTs>
class GenericDictionary : public mixins::iDuplicable< iBaseValue<AspectTs...>
                                                    , GenericDictionary<KeyT, AspectTs ...>
                                                    , TValue< Hash<KeyT, iBaseValue<AspectTs...> * >
                                                            , AspectTs... >
                                                    >
                         , public Traits<AspectTs...>::template DictionaryAspect<KeyT> {
public:
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
    /// Protected entry-insertion method.
    virtual std::pair<typename Hash<KeyT, iBaseValue<AspectTs...> *>::iterator, bool>
    _insert_parameter( const KeyT & k, iBaseValue<AspectTs...> * p ) {
        return this->_mutable_value().emplace(k, p);
    }
public:
    /// Common ctr, that barely forwards arguments to aspect ctr.
    template<typename ... CtrArgTs>
    explicit GenericDictionary( CtrArgTs ... ctrArgs ) : Traits<AspectTs...> \
                                ::template DictionaryAspect<KeyT>(ctrArgs ...) {}

    virtual ~GenericDictionary() {
        for( auto p : this->_mutable_value() ) {
            _free( p.second );
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

