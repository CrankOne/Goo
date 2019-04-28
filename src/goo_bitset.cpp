# include "goo_bitset.hpp"
# include "goo_exception.hpp"

# include <limits>

namespace goo {

Bitset::Bitset() : _size(0) {}

Bitset::~Bitset() {
    if(!empty()) {
        delete [] _data;
    }
}

Bitset::Bitset(size_t length) : _size(0) {
    resize( length );
}

void
Bitset::set() {
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] = Word_t(~Word_t{0});
    }
}

void
Bitset::set(size_t n, bool value) {
    if(!value) {
        reset(n);
        return;
    }
    size_t wd = n/nBiW
         , bd = n%nBiW
         ;
    _data[wd] |= (Word_t{1} << ((nBiW - 1) - bd));
}

void
Bitset::reset() {
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] ^= _data[nw];
    }
}

void
Bitset::reset(size_t n) {
    size_t wd = n/nBiW
         , bd = n%nBiW
         ;
    _data[wd] &= ~((Word_t{1} << bd));
}

void
Bitset::resize( size_t newSize ) {
    size_t newNWords = newSize/nBiW
         , remnant = newSize%nBiW
         ;
    Word_t newTailMask = 0x0;
    ++newNWords;
    // Set least-significant n bits to 0, others to 1 to obtain a tail
    // bit mask.
    newTailMask = std::numeric_limits<Word_t>::max();
    if( remnant ) {
        newTailMask &= ~((Word_t(1) << (nBiW - remnant)) - 1);
    }
    if( empty() ) {
        _data = new Word_t [newNWords];
        _size = newSize;
        _nWords = newNWords;
        _tailMask = newTailMask;
    } else {
        emraise( unimplemented, "TODO: preserve values in bitset on realloc." )
    }
}

std::string
Bitset::to_string() const {
    std::string s(size(), '\0');
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        for( size_t nb = 0; nb < nBiW; ++nb ) {
            s[nw*nBiW + nb] = ((Word_t(1) << nb) & _data[nw] ? '1' : '0' );
        }
    }
    for( size_t nb = 0; nb < _size%nBiW; ++nb ) {
        s[(_nWords-1)*nBiW + nb] = ((Word_t(1) << (nBiW - nb - 1)) & _data[_nWords-1] ? '1' : '0' );
    }
    return s;
}

bool
Bitset::all() const {
    // todo: won't it be more efficient to use flip().none() instead?
    assert(!empty());
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        if( (Word_t) ~(_data[nw]) ) {
            return false;
        }
    }
    return 0x0 == ~(_data[_nWords-1] | (~_tailMask));
}

bool
Bitset::any() const  {
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        if( _data[nw] ) {
            return true;
        }
    }
    return (Word_t) (_data[_nWords-1] & _tailMask);
}

bool
Bitset::none() const {
    assert(!empty());
    return !any();
}

# if 1
void
Bitset::dump( std::ostream & os ) {
    if(empty()) return;
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        for( size_t nb = 0; nb < nBiW; ++nb ) {
            os << ((Word_t(1) << nb) & _data[nw] ? '1' : '0' );
        }
        os << " ";
    }
    for( size_t nb = 0; nb < _size%nBiW; ++nb ) {
        os << ((Word_t(1) << nb) & _data[_nWords-1] ? '1' : '0' );
    }
    // XXX:
    os << ", _size = " << _size
       << ", _nWords = " << _nWords
       << ", _tailMask = " << std::hex << (int) _tailMask << std::dec << "(" << (int) _tailMask << ")"
       << std::endl;
    os << "std::string -> " << to_string() << std::endl;
}
# endif

}  // namespace goo

