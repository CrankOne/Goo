# include <cstdint>
# include <cstddef>
# include <ostream>
# include <cassert>
# include <cstring>
# include <stdexcept>
# include <bitset>

# pragma once

namespace goo {

/**@class Bitset
 * @brief A dynamic bitset implementaation.
 *
 * We want some nice features of bitsets which length may be parameterised
 * once during the runtime (or supposed to undergo rare resizes in non-critical
 * sections). We don't want to bring the boost dependencies, however.
 *
 * It has to be similar to std::vector<>, but optimized rather for single
 * allocation, rather than utilize some pro-active allocating strategies.
 *
 * We also want some synctactic sugar, like in std::bitset<N> (all(), none(),
 * binary operators, etc.).
 *
 * It may be wiped in future, if STL will introduce such a thing... Perhaps,
 * for the time being the bitset is a nice container to study for novice.
 *
 * @TODO: profile, study performance against different Word_t types
 * @TODO: allocators
 */
class Bitset {
protected:
    typedef uint32_t Word_t;
    constexpr static size_t nBiW = 8*sizeof(Word_t);
private:
    size_t _size;
    Word_t * _data;

    size_t _nWords;  // real physical size of the _data
    Word_t _tailMask;
protected:
    virtual Word_t * _alloc( size_t );
    virtual void _delete( Word_t * );

    void _free();
public:
    /// Empty set ctr.
    Bitset();
    /// Bitset copy ctr.
    Bitset( const Bitset & );
    /// Allocates set of size N.
    Bitset(size_t length);
    /// Frees allocated memory.
    ~Bitset();
    /// Assignment operator.
    Bitset & operator=(const Bitset&);
    /// Sets all bits to true.
    void set();
    /// Sets n-th bit to `value'.
    void set(size_t n, bool value=true);
    /// Sets all bits to false.
    void reset();
    /// Sets n-th bit to false.
    void reset(size_t n);
    /// Re-allocates the bitset. New bits will be appended to the back in
    /// undefined state.
    void resize(size_t);
    /// Inverts n-th bit in set, returns *this ref.
    Bitset & flip(size_t);

    /// Returns true if bitset is empty (of zero size).
    bool empty() const { return !_size; }  // todo: suboptimal?
    /// Returns number of bits stored.
    size_t size() const { return _size; }

    /// Returns value of n-th bit.
    inline bool test(size_t n) const {
        assert( size() > n );
        return _data[n/nBiW] & (Word_t{1} << (n%nBiW));
    }
    /// Returns true, if all bits are set to truth.
    bool all() const;
    operator bool() const { return any(); }
    /// Returns true if any of bits is set to truth.
    bool any() const;
    /// Returns true if none of bits is set to truth.
    bool none() const;

    /// Inverts all bits in set, returns *this ref.
    Bitset & flip();
    Bitset operator~() const;
    /// Computes bitwise-and with given bitset, writing result to current set.
    Bitset & bitwise_and( const Bitset & );
    Bitset & operator&=( const Bitset & bs) { return bitwise_and(bs); }
    Bitset operator&( const Bitset & ) const;
    /// Computes bitwise-or with givine bitset, writing result to current set.
    Bitset & bitwise_or( const Bitset & );
    Bitset & operator|=( const Bitset & bs) { return bitwise_or(bs); }
    Bitset operator|( const Bitset & ) const;
    /// Computes bitwise-exclusive-or with givine bitset, writing result to
    /// current set.
    Bitset & bitwise_xor( const Bitset & );
    Bitset & operator^=( const Bitset & bs) { return bitwise_xor(bs); }
    Bitset operator^( const Bitset & ) const;

    /// Template method performing bitwise conversion to certain type.
    template<typename T> T to() const;
    /// Returns textual representation of the bitset as a sequence of 1 and 0.
    std::string to_string() const;
    /// Returns unsigned long representation of bitset.
    unsigned long to_ulong() const { return to<unsigned long>(); }
    /// Returns unsigned long long representation of bitset.
    unsigned long long to_ullong() const { return to<unsigned long long>(); }

    friend std::ostream & operator<<( std::ostream & os, const Bitset & bs ) {
        os << bs.to_string(); return os; }

    //void dump( std::ostream &);  // XXX: for debugging
};

template<typename T> T
Bitset::to() const {
    if( sizeof(T) < _nWords*sizeof(Word_t) )
        throw std::overflow_error("Bitset is too large.");
    T result;
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        memcpy( ((Word_t*) &result) + nw
              , _data + nw
              , sizeof(Word_t) );
    }
    *(((Word_t*) &result) + (_nWords-1)) &= _tailMask;
    return result;
}

template<> std::string
Bitset::to<std::string>() const {
    return to_string();
}

# if 0
template<size_t N> std::bitset<N>
Bitset::to<std::bitset<N>>() const {
    // ...
}
# endif

}  // namespace goo

