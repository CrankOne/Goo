
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

// Created by crank on 12.01.18.

# ifndef H_GOO_PARAMETERS_BASES_H
# define H_GOO_PARAMETERS_BASES_H

# include <vector>
# include <list>
# include <map>

/**@file bases.hpp
 * Sequence container templates. The particular selection may significantly
 * affect performance or memory fragmentation. Since goo::dict is generally
 * designed for application configuration that occurs once, the performance
 * matters less than memory consumption that is slightly more efficient for
 * list sequences (TODO: has to be checked on benchmarks, actually).
 */

namespace goo {
namespace dict {

/// Type referencing element position in a List.
typedef unsigned long ListIndex;

/// Array parameter container template. Used to store the homogeneous typed
/// singular parameters.
template<typename T> using Array = std::vector<T>;

/// List of heterogeneous parameters kept within the section or "list of
/// structres".
template<typename T> using List = std::list<T>;

/// This container is used to cache parameters indexing.
template<typename KeyT, typename ValueT> using Hash = std::map<KeyT, ValueT>;

namespace aux {

/// @brief This routine performs simple token extraction from head of the option
/// path.
///
/// For example, the following string:
///     "one.three-four.five"
/// will be split in following manner:
///     current = "one"
///     tail = "three-four.five".
/// In case when only one token is provided, it will be written
/// in `current', and the tail will be empty.
///
/// No data will be allocated on heap, but path string will be rewritten.
///
/// Throws goo::TheException<badParameter> if characters not allowed by
/// goo::dict path specification is found (alnum + '-', '_' for tokens and
/// '.' as a separator). Note, that after throwing an exception
/// the path argument pointer reference will point to the "bad" character.
///
/// The idx reference is for digits-only rokens. They will be parsed and
/// special value will be returned.
///
/// One may perform easy check for latest token extraction with bitwise-and
/// operation using the return result code ("rc & 0x2 == false" indicates
/// path depletion).
///
/// @returns 0 if no token can be extracted and current token is a string
/// @returns 1 if no token can be extracted and current token is an index
/// @returns 2 if there are something in path after extraction and current
///            token is a string
/// @returns 3 if there are something in path after extraction and current
///            token is an index
/// @param path the mutable path string expression
/// @param current the target pointer that will refer to extracted token
///        start.
int pull_opt_path_token( char *& path
                              , char *& current
                              , long & idx );

}  // namespace aux

}  // namespace dict
}  // namespace goo


#endif //GOO_BASES_HPP
