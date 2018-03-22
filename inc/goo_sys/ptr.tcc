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

// Created by crank on 08.03.18.

# ifndef H_GOO_GENERIC_POINTER_H
# define H_GOO_GENERIC_POINTER_H

# include <limits>
# include <cstddef>

namespace goo {
namespace dict {

/**\brief A shimmering template composite pointer class.
 * \class Ptr
 *
 * This template class establishes a connection between data structure
 * referencing object of certain type (typically dynamically allocated) and
 * special traits template instance providing common basis for classic
 * pointer-like syntax.
 *
 * These pointers are useful for number of situations when C++ struct has to be
 * seamlessly mapped onto some physical layout (continious blocks, hierarchical
 * allocators, shared memory segments, etc).
 *
 * \ingroup mem
 * */
template< typename T
        , template<typename> typename TraitsT> class GenericPointer {
public:
    typedef TraitsT<T> Traits;
    typedef Traits::Distance Distance;
private:
    Traits::ID _id;
public:
    /// Default ctr. Has to construct uninitialized generic pointer instance.
    GenericPointer() = default;
    /// Copy ctr, copying pointer ID value.
    GenericPointer( const GenericPointer & orig ) = default;
    /// Dedicated nullptr ctr, constructing instance initialized to special
    /// value.
    GenericPointer( nullptr_t ) : _id(nullptr) {}

    //
    // Basic accessing functions

    /// Returns absolute C/C++ pointer to the entry.
    T * c_ptr() {
        const_cast<T *>( ((const GenericPointer *) this)->c_ptr() );
    }

    /// Returns absolute C/C++ pointer to the entry.
    operator T * () const { return c_ptr(); }

    /// Returns absolute C/C++ pointer to the entry (const).
    const T * c_ptr() const {
        return Traits::to_ptr( _id );
    }

    /// Returns absolute C/C++ pointer to the entry (const).
    operator const T * () const { return c_ptr(); }

    //
    // Pointer comparison operators

    /// Entry referring operator (mutable).
    T * operator->() {
        return c_ptr();
    }
    /// Entry referring operator (const).
    const T * operator->() const {
        return c_ptr();
    }
    /// Entry deeferencing operator (mutable).
    T & operator*() {
        return *c_ptr();
    }
    /// Entry deeferencing operator (const).
    const T & operator*() const {
        return *c_ptr();
    }

    // Depending on, whether the _Goo_m_STRICT_DICT_POINTERS macro is defined,
    // these comparison operators may or may not also check that allocators of
    // both "pointers" are the same. Basically, architecture has to exclude
    // the sitauations when gt/lt operators are applied for "pointers" from
    // different pools, however we have to envisage this additional check to
    // be on the safe side. gte/lte operators are inferred.

    bool operator> (const GenericPointer & r) const {
        return Traits::gt( _id, r._id );
    }
    bool operator< (const GenericPointer & r) const {
        return Traits::lt( _id, r._id );
    }
    /// Inequality operator considers comparison of two pointers from different
    /// affiliation as a valid case.
    bool operator!= (const GenericPointer & r) const {
        return Traits::ne( _id, r._id );
    }

    bool operator!= (nullptr_t) const {
        return Traits::nonnull( _id );
    }


    bool operator<= (const GenericPointer & o) const {
        return !(*this > o);
    }
    bool operator>= (const GenericPointer & o) const {
        return !(*this < o);
    }
    bool operator== (const GenericPointer & o) const {
        return !(*this != o);
    }
    bool operator== (nullptr_t) const {
        return !(*this != nullptr);
    }

    //
    // Pointer arithmetic operations
    GenericPointer & operator++ () {
        _id = Traits::inc( _id );
    }
    GenericPointer & operator-- () {
        _id = Traits::dec( _id );
    }
    GenericPointer & operator-=(Distance n) {
        _id = Traits::advance( _id, -n );
        return *this;
    }
    GenericPointer & operator+=(size_t n) {
        _id = Traits::advance( _id, n );
        return *this;
    }
    Distance operator-(const GenericPointer & r) const {
        return Traits::distance( _id, r._id );
    }

    GenericPointer operator-(Distance n) {
        return GenericPointer(Traits::advance( -n ));
    }
    GenericPointer operator+(Distance n) {
        return GenericPointer(Traits::advance(  n ));
    }
    GenericPointer operator++ (int) {
        return GenericPointer( ++ (*this) );
    }
    GenericPointer operator-- (int) {
        return GenericPointer( -- (*this) );
    }
};

}  // namespace ::goo::dict
}  // namespace ::goo

# endif  // H_GOO_GENERIC_POINTER_H
