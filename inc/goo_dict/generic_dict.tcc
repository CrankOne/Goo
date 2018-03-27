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
                                , typename AllocatorT<iReferable>::pointer
                                , std::hash<KeyT>
                                , std::equal_to<KeyT>
                                , AllocatorT< std::pair<const KeyT, typename AllocatorT<iReferable>::pointer> > >;
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
 * @tparam KeyT Key type indexing the references.
 * @tparam X Template referable class.
 * @tparam Map Template container performing actual mapping.
 */
template< typename KeyT
        , template <typename> class X
        , template <typename> class AllocatorT=std::allocator >
class Dictionary : public X< typename IndexingTraits<KeyT, X>::template Map< AllocatorT > > {
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
    typedef X< typename IndexingTraits<KeyT, X>::template Map< AllocatorT > > TheMap;
public:
    /// Ctr forwarding args to Map class defined by IndexingTraits<KeyT, X>::Map.
    template<typename ... CtrArgTs>
    Dictionary(CtrArgTs && ... ctrArgs) : TheMap( ctrArgs ... ) {}
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
                                 , typename DictionaryT::template Allocator<iReferable>
                                                       ::pointer
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

