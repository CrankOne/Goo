
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

/// Array parameter container template. Used to store the homogeneous typed
/// singular parameters.
template<typename T> using Array = std::vector<T>;

/// List of heterogeneous parameters kept within the section or "list of
/// structres".
template<typename T> using List = std::list<T>;

/// This container is used to cache name-indexed parameters.
template<typename ValueT> using NameHash = std::map<std::string, ValueT>;

/// This container is used to cache shortcut-indexed parameters.
template<typename ValueT> using CharsHash = std::map<char, ValueT>;

}  // namespace dict
}  // namespace goo


#endif //GOO_BASES_HPP
