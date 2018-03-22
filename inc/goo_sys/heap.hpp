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

//
// Created by crank on 20.03.18.
//

# ifndef H_GOO_SYS_MEM_HEAP_H
# define H_GOO_SYS_MEM_HEAP_H

# include <cstddef>

namespace goo {
namespace mem {

/// Traits for plain pointer. To be used within GenericPointer template.
template<typename T>
struct PlainPointerTraits {
    /// Identifier data type, referencing instance in memory.
    typedef T* ID;
    /// Distance between two pointers, in number of entries.
    typedef size_t Distance;

    /// Translates identifier to C-pointer.
    static const T * to_ptr( T * p ) const { return p; }
    /// Returns true if greater than.
    static bool gt( const T * l, const T * r ) { return l > r; }
    /// Returns true if lesser than.
    static bool lt( const T * l, const T * r ) { return l < r; }
    /// Returns true if not equal.
    static bool ne( const T * l, const T * r ) { return l != r; }
    /// Returns true if not null.
    static bool nonnull( const T * p ) { return p != nullptr; }
    /// Returns increment result.
    static T * inc( const T * p ) { return ++p; }
    /// Returns decrement result.
    static T * dec( const T * p ) { return --p; }
    /// Returns advance (p + n) result.
    static T * advance( const T * p, size_t n ) { return p + n; }
};

}  // namespace mem
}  // namespace goo

# endif  // H_GOO_SYS_MEM_HEAP_H
