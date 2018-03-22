
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

# ifndef H_GOO_DICT_TYPES_H
# define H_GOO_DICT_TYPES_H

# include "goo_config.h"

# include <vector>
# include <list>
# include <unordered_map>
# include <unordered_set>
# include <limits>

# include "goo_sys/exception.hpp"

# ifdef FNTHN_ALLOCATORS
    // TODO: use https://github.com/foonathan/memory here
# endif

// Fine configuration for
// - disables dictionaries (overall):
//# define _Goo_m_DISABLE_DICTIONARIES
// - application configuration dictionaries implementation
# define _Goo_m_DISABLE_APP_CONF_DICTIONARIES
// - disables common dictionaries implementation
# define _Goo_m_DISABLE_GENERIC_DICTIONARIES

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

/**@file bases.hpp
 * Sequence container templates. The particular selection may significantly
 * affect performance or memory fragmentation. Since goo::dict is generally
 * designed for application configuration that occurs once, the performance
 * matters less than memory consumption that is slightly more efficient for
 * list sequences (TODO: has to be checked on benchmarks, actually).
 */

namespace goo {
namespace dict {

struct AbstractValueAllocator {
    /// Called by insertion proxies to allocate data of various types, including
    /// instances of particular parameter types.
    template<typename T, typename ... CtrArgTs>
    T *_alloc(CtrArgTs ... args) /*throw(TheException<Exception::memAllocError>)*/ {
        // TODO: use more advanced allocation logic here.
        return new T(args...);
    }

    template<typename T>
    void _free(T *p) /*throw(TheException<Exception::memAllocError>)*/ {
        // TODO: use more advanced allocation logic here.
        delete p;
    }
    // ...
};

/// Alias type for STL containers referring to the generic dictionary allocator
/// instance.
template<typename T>
class TheAllocatorHandle {
protected:
    AbstractValueAllocator & _t;
public :
    // typedefs
    typedef T value_type;
    typedef value_type* pointer;
    typedef const value_type* const_pointer;
    typedef value_type& reference;
    typedef const value_type& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
public:
    // convert an allocator<T> to allocator<U>
    template<typename U>
    struct rebind {
        typedef TheAllocatorHandle<U> other;
    };
public:
    TheAllocatorHandle() = delete;
    inline TheAllocatorHandle( AbstractValueAllocator & a ) : _t(a) {}
    inline ~TheAllocatorHandle() {}
    inline TheAllocatorHandle( const TheAllocatorHandle & o ) : _t(o._t) {}
    template<typename U>
    inline explicit TheAllocatorHandle( const TheAllocatorHandle<U> & o ) : _t(o._t) {}

    // address
    inline pointer address(reference r) { return &r; }
    inline const_pointer address(const_reference r) { return &r; }

    // memory allocation
    inline pointer allocate(size_type cnt,
            typename std::allocator<void>::const_pointer = 0) {
        return reinterpret_cast<pointer>(::operator new(cnt * sizeof (T)));
    }
    inline void deallocate(pointer p, size_type) {
        ::operator delete(p);
    }

    // size
    inline size_type max_size() const {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    // construction/destruction
    inline void construct(pointer p, const T& t) { new(p) T(t); }
    inline void destroy(pointer p) { p->~T(); }

    inline bool operator==(TheAllocatorHandle const&) { return true; }
    inline bool operator!=(TheAllocatorHandle const& a) { return !operator==(a); }
};    //    end of class Allocator

/**@brief Hierarchical dictionary allocator class
 *
 * Incapsulates allocation of every dictionary-related entity in a way allowing
 * to reside the complex hierarchical data within controllable memory blocks
 * (e.g. allocated within shmat()).
 *
 * In this approach, the allocators aren't necessary static types. It may be an
 * instance, for example, controlling the memory block of fixed size.
 */
template<typename ... AspectTs>
class DictionaryAllocator : public AbstractValueAllocator {
protected:
    // ...
public:
};

/// Type referencing element position in a List.
typedef unsigned long ListIndex;

/// Array parameter container template. Used to store the homogeneous typed
/// singular parameters.
template<typename T> using Array = std::vector< T, TheAllocatorHandle<T> >;

/// List of heterogeneous parameters kept within the section or "list of
/// structres".
template<typename T> using List = std::list< T, TheAllocatorHandle<T> >;

/// This container is used to cache parameters indexing (associative array).
template<typename KeyT, typename ValueT> using Hash = std::unordered_map< KeyT
                                             , ValueT
                                             , std::hash<KeyT>
                                             , std::equal_to<KeyT>
                                             , TheAllocatorHandle<std::pair<KeyT, ValueT> > >;

template<typename ValueT> using Set = std::unordered_set< ValueT
                                       , std::less<ValueT>
                                       , TheAllocatorHandle<ValueT> >;

typedef std::basic_string< char
                         , std::char_traits<char>
                         , TheAllocatorHandle<char>
                         > String;



}  // namespace ::goo::dict
}  // namespace ::goo

namespace std {

template<>
struct hash<goo::dict::String> {
    size_t operator()( const goo::dict::String & k ) const {
        return std::_Hash_impl::hash(k.data(), k.length());
    }
};

}  // namespace ::std

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)

# endif  // H_GOO_DICT_TYPES_H
