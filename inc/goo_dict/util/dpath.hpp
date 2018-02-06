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
 * Created by crank on 16.01.18.
 */

#ifndef GOO_PARAMETERS_DPATH_HPP
#define GOO_PARAMETERS_DPATH_HPP

# include "goo_dict/parameters/los.hpp"
# include "goo_dict/generic_dict.tcc"

# include <cstring>

namespace goo {
namespace dict {
namespace utils {

/** @brief This routine performs simple token extraction from head of the option
 * path.
 *
 * For example, the following string:
 *     "one.three-four.five"
 * will be split in following manner:
 *     current = "one"
 *     tail = "three-four.five".
 * In case when only one token is provided, it will be written
 * in `current', and the tail will be empty.
 *
 * No data will be allocated on heap, but path string will be rewritten.
 *
 * Throws goo::TheException<badParameter> if characters not allowed by
 * goo::dict path specification is found (alnum + '-', '_' for tokens and
 * '.' as a separator). Note, that after throwing an exception
 * the path argument pointer reference will point to the "bad" character.
 *
 * The idx reference is for digits-only rokens. They will be parsed and
 * special value will be returned.
 *
 * One may perform easy check for latest token extraction with bitwise-and
 * operation using the return result code ("rc & 0x2 == false" indicates
 * path depletion).
 *
 * Note: empty tokens (i.e. ".." or ".#." ) will be recognized as valid (but
 * empty) tokens as well.
 *
 * @returns 0 if no token can be extracted and current token is a string
 * @returns 1 if no token can be extracted and current token is an index
 * @returns 2 if there are something in path after extraction and current
 *           token is a string
 * @returns 3 if there are something in path after extraction and current
 *           token is an index
 * @param path the mutable path string expression
 * @param current the target pointer that will refer to extracted token
 *        start.
 */
int pull_opt_path_token( char *& path
                      , const char *& current
                      , ListIndex & idx );

/// Intermediate representation of dictionary/list index referencing an element.
struct DictPath {
    union {
        const char * name;
        ListIndex index;
    } id;
    bool isIndex;
    DictPath * next;

    DictPath() : next(nullptr) { id.name = nullptr; }
    DictPath( const char * strID ) : next(nullptr) { id.name = strID; isIndex = false; }
    DictPath( ListIndex idx ) : next(nullptr) { id.index = idx; isIndex = true; }

    //template<typename T> typename T::Key key_for();
};

# if 0
template<> inline AppCfgTraits::Dictionary::Key
DictPath::key_for<AppCfgTraits::Dictionary>() {
    if( isIndex ) {
        emraise( badCast, "Path token is an integer index =%ld while"
            " string key requested.", id.index );
    }
    return id.name;
}

//template<> inline typename AppCfgTraits::ListOfStructures::Key
//DictPath::key_for<DictionaryParameter>() {
//    return key_for<Dictionary>();
//}

template<> inline typename AppCfgTraits::ListOfStructures::Key
DictPath::key_for<AppCfgTraits::ListOfStructures>() {
    if( !isIndex ) {
        emraise( badCast, "Path token is a string key =\"%s\" while"
            " integer index requested.", id.name );
    }
    return id.index;
}
# endif

//template<> inline typename LoSParameter::Key
//DictPath::key_for<LoSParameter>() {
//    return key_for<ListOfStructures>();
//}

/** @brief System dict-path parsing routine.
 *
 * This is an auxiliary routine optimized rather for efficiency. It produces
 * a linked list of path tokens to be further used mainly by querying/insertion
 * routines of insertion proxy objects.
 *
 * It will perform sequential extraction of path tokens from given string
 * producing a linked list containing a DictPath instances. To prevent cache
 * inflation and, thus, hopefully increase efficiency a little bit, all the
 * nodes have to be stacked into the pToks array.
 *
 * Note, that provided pathString parameter will be corrupted after evaluation in
 * the same way pull_opt_path_token() does it.
 *
 * @param pathString mutable string that will be tokenized.
 * @param pToks pool of tokens available for use.
 * @param nPToks number of available tokens in pool.
 * @return number of tokens.
 */
size_t parse_dict_path( char * pathString
                     , DictPath * pToks
                     , size_t nPToks );

# ifndef _Goo_m_PATH_TOKENS_START_2PWR
/// Macros is an integer constant, the power of two referencing the maximum number
/// of tokens that may be used in single entry referencing invocation. See also
/// _Goo_m_MAX_PATH_TOKENS_2PWR.
#   define _Goo_m_PATH_TOKENS_START_2PWR 5
# endif  // _Goo_m_PATH_TOKENS_START_2PWR

# ifndef _Goo_m_MAX_PATH_TOKENS_2PWR
/// Macro is an int constant, the power of 2 indicating maximum number of path
/// tokens that may be used in single invocation. Has to be grater than
/// _Goo_m_PATH_TOKENS_START_2PWR
#   define _Goo_m_MAX_PATH_TOKENS_2PWR 10
# endif  // _Goo_m_MAX_PATH_TOKENS_2PWR

# if 0
// NOTE: this code snippet may be usefult to significantly enhance the
// performance of entry retrieval from the dictionaries. However, there is
// a possible danger of stack overflow while using the alloca() within the
// template functions since the former are always inline. See:
//  https://stackoverflow.com/a/3410689/1734499
// for details. Standing on the safe side, we keep the heap allocation for
// now.
std::pair< char *
        , std::vector<DictPath> >
dpath( const std::string & path ) {
    char * pDup = alloca( path.size() + 1 );
    utils::DictPath * toksPool;
    std::vector<utils::DictPath> dp;
    for( size_t nToksPool = (1 << _Goo_m_PATH_TOKENS_START_2PWR)
       ; nToksPool < (1 << _Goo_m_MAX_PATH_TOKENS_2PWR)
       ; nToksPool <<= 2 ) {
        toksPool = alloca( nToksPool );
        size_t nToks = utils::parse_dict_path( pDup, toksPool, nToksPool );
        if( nToksPool >= (1 << _Goo_m_MAX_PATH_TOKENS_2PWR) ) {
            emraise( badParameter
                   , "Entry path is too long to be parsed. nToksPool=%lu exceeded"
                     " maximum.", nToksPool );
        }
        if( nToks >= nToksPool ) {
            continue;  // double the pool.
        }
        // We have to keep this invocation inside the for(){} scope to preserve
        // alloca()
        dp.reserve( nToks );
        for( DictPath * pt = toksPool; pt; pt = pt->next ) {
            dp.push_back( *pt );
            _TODO_  // TODO: rewrite ->next pointer
        }
    }
    _TODO_  // NOTE: all the names will be freed after leaving this scope, so
            // consider another on-stack trickery...
    return dp;
}
# endif

/// Hides all the micromanagement details of parse_dict_path() within itself
/// providing an easy interface to turning stringified paths into the DictPath
/// list located in single contiguous memory block.
std::vector<DictPath> dpath( const std::string & path );

}  // namespace utils
}  // namespace dict
}  // namespace goo

#endif  // GOO_PARAMETERS_DPATH_HPP
