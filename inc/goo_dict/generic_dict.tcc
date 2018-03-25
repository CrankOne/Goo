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

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

namespace goo {
namespace dict {

# if 0  // shall be defined within the allocator type
    /// Pointer type referencing particular referable instance within
    /// dictionary.
    typedef iReferable * Reference;
# endif

template< typename KeyT
        , template <typename> class X
        >
struct IndexingTraits {
    /// Container type.
    template< template <typename> class AllocatorT > using Map
            = std::unordered_map< KeyT
                                , typename AllocatorT<iReferable>::pointer
                                , std::hash<KeyT>
                                , std::equal_to<KeyT>
                                , AllocatorT< std::pair<const KeyT, typename AllocatorT<iReferable>::pointer> > >;
};

/**\brief Dictionary is an object defining mapping between keys and referables.
 *
 * @tparam KeyT Key type indexing the references.
 * @tparam X Template referable class.
 * @tparam Map Template container performing actual mapping.
 */
template< typename KeyT
        , template <typename> class X
        , template <typename> class AllocatorT >
class Dictionary : public X< typename IndexingTraits<KeyT, X>::template Map< AllocatorT > > {
public:
    template<typename ... CtrArgTs>
    Dictionary(CtrArgTs ... ctrArgs) : X< typename IndexingTraits<KeyT, X> \
                                ::template Map< AllocatorT > > ( ctrArgs ... ) {}
};  // class Dictionary

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_GENERIC_DICT_H

