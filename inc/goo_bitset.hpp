# include <cstdint>
# include <cstddef>
# include <ostream>
# include <cassert>

# pragma once

namespace goo {

/// Dynamic bitset.
///
/// We want some nice features of bitsets which length may be parameterised
/// once during the runtime. We don't want to bear the boost dependencies,
/// however.
/// It has to be similar to std::vector<>, but optimized rather for single
/// allocation, rather than utilize some pro-active allocating strategies.
/// We also want some synctactic sugar.
/// It may be wiped in future, if STL will introduce such a thing...
class Bitset {
protected:
    typedef uint8_t Word_t;
    constexpr static size_t nBiW = 8*sizeof(Word_t);
private:
    size_t _size;
    Word_t * _data;

    size_t _nWords;  // real physical size of the _data
    Word_t _tailMask;
public:
    Bitset();
    Bitset(size_t length);
    ~Bitset();

    /// Sets all bits to true.
    void set();
    /// Sets n-th bit to `value'.
    void set(size_t n, bool value=true);
    /// Sets all bits to false.
    void reset();
    /// Sets n-th bit to false.
    void reset(size_t n);
    /// Returns true if bitset is empty (of zero size).
    bool empty() const { return !_size; }  // todo: suboptimal?
    /// Returns number of bits stored.
    size_t size() const { return _size; }
    /// Re-allocates the bitset. New bits will be appended to the back in
    /// undefined state.
    void resize(size_t);
    /// Returns textual representation of the bitset.
    std::string to_string() const;

    /// Returns value of n-th bit.
    bool test(size_t n) const {
        assert( size() > n );
        return _data[n/nBiW] & (1 << (nBiW - n%nBiW - 1));
    }
    /// Returns true, if all bits are set to truth.
    bool all() const;
    /// Returns true if any of bits is set to truth.
    bool any() const;
    /// Returns true if none of bits is set to truth.
    bool none() const;

    friend std::ostream & operator<<( std::ostream & os, const Bitset & bs ) {
        os << bs.to_string(); return os; }

    void dump( std::ostream &);
};

}  // namespace goo

