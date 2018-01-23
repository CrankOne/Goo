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
struct GenericDictionary;

// Common traits (pretty basic ones)
template< typename ... AspectTs >
struct Traits {
    // One probably will want to append the dictionary traits (within partial
    // specialization). By default, each dictionary aspect keeps a map
    // indexing a set of self-typed instance pointers by same key.
    template<typename KeyT> struct DictionaryAspect :
            public TValue< Hash<KeyT, GenericDictionary<KeyT, AspectTs...>* > >
            // ... public Value< Map< otherKeyT , Dictionary<KeyT, AspectTs...>* > >
    { };
};

template< typename KeyT
        , typename ... AspectTs>
struct GenericDictionary : public mixins::iDuplicable< iBaseValue<AspectTs...>
                                                   , GenericDictionary<KeyT, AspectTs ...>
                                                   , TValue< Hash<KeyT, iBaseValue<AspectTs...> * >, AspectTs... >
                                                   >
                         , public Traits<AspectTs...>::template DictionaryAspect<KeyT> {

    //template< typename
    //        , template <class> class
    //        , class ... > friend class InsertionProxy;
    std::pair<typename Hash<KeyT, iBaseValue<AspectTs...> *>::iterator, bool>
    insert_parameter( const KeyT & k, iBaseValue<AspectTs...> * p ) {
        return this->_mutable_value().emplace(k, p);
    }

    template<typename ... CtrArgTs>
    explicit GenericDictionary( CtrArgTs ... ctrArgs ) : Traits<AspectTs...> \
                                ::template DictionaryAspect<KeyT>(ctrArgs ...) {}

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

