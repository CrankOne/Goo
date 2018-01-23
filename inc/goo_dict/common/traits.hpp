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

/*
 * Created by crank on 23.01.18.
 */

#ifndef GOO_TRAITS_HPP
#define GOO_TRAITS_HPP

# include "goo_dict/generic_dict.tcc"
# include "goo_dict/common_aspects.hpp"

namespace goo {
namespace dict {

/// Specialization for common name-indexed dictionary structure used for
/// generic configurations. Provides list-like structures, but is not supposed
/// to store shortcuts.
template<>
struct Traits< aspects::Description
            , aspects::iStringConvertible
            , aspects::IsSet
            , aspects::Array
            > {
    template<typename KeyT> class DictionaryAspect;
};

typedef Traits< aspects::Description
              , aspects::iStringConvertible
              , aspects::IsSet
              , aspects::Array
              > GenericConfTraits;

/// Named dictionary config template specialization.
template<>
class GenericConfTraits::DictionaryAspect<std::string>
        : public Hash< std::string
                     , GenericDictionary< std::string
                                       , aspects::Description
                                       , aspects::iStringConvertible
                                       , aspects::IsSet
                                       , aspects::Array
                                       > *
                     >
        , public Hash< std::string
                     , GenericDictionary< ListIndex
                                       , aspects::Description
                                       , aspects::iStringConvertible
                                       , aspects::IsSet
                                       , aspects::Array
                                       > *
                     >
        , public aspects::Description {
public:
    explicit DictionaryAspect( const std::string & d ) : aspects::Description( d ) {}
};

/// Integer-indexed (List-of-Structures) dictionary config template
/// specialization.
template<>
class GenericConfTraits::DictionaryAspect<ListIndex>
        : public Hash< ListIndex
                     , GenericDictionary< std::string
                                       , aspects::Description
                                       , aspects::iStringConvertible
                                       , aspects::IsSet
                                       , aspects::Array
                                       > *
                     >
        , public Hash< ListIndex
                     , GenericDictionary< ListIndex > *  // (!) has no aspects
                     >
        , public aspects::Description {
public:
    DictionaryAspect( const std::string & d ) : aspects::Description(d) {}
};

}  // ns dict
}  // ns goo

#endif  // GOO_TRAITS_HPP
