# include "goo_bitset.hpp"
# include "goo_exception.hpp"

# include <limits>

namespace goo {

Bitset::Bitset() : _size(0) {}

Bitset::Bitset( const Bitset & o ) : _size(o._size)
                                   , _nWords(o._nWords)
                                   , _tailMask(o._tailMask) {
    _data = _alloc(o._nWords);
    memcpy(_data, o._data, _nWords);
}

Bitset::Bitset(size_t length) : _size(0) {
    if(length) resize( length );
}

Bitset::Bitset(size_t length, unsigned long v) : Bitset(length) {
    if( length <= 8*sizeof(unsigned long) ) {
        reset();
        for( size_t i = 0; i < size(); ++i ) {
            set( i, (bool) (v & (1 << i)) );
        }
    } else {
        _TODO_
    }
}

Bitset::Word_t *
Bitset::_alloc( size_t nw ) {
    # if 0
    auto d = new Word_t [nw];
    bzero( d, sizeof(Word_t)*nw );
    return d;
    # else
    return new Word_t [nw];
    # endif
}

void
Bitset::_delete( Word_t * ptr ) {
    delete [] ptr;
}

void
Bitset::_free() {
    if(!empty()) {
        _size = 0;
        _delete(_data);
    }
}

Bitset::~Bitset() {
    _free();
}

Bitset &
Bitset::operator=(const Bitset & o) {
    this->resize(o._size);
    memcpy( _data, o._data, _nWords*sizeof(Word_t) );
    return *this;
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
    _data[wd] |= (Word_t{1} << bd);
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
    if(!newSize) {
        _free();
        return;
    }
    size_t newNWords = newSize/nBiW
         , remnant = newSize%nBiW
         ;
    Word_t newTailMask = 0x0;
    ++newNWords;
    // Set least-significant n bits to 0, others to 1 to obtain a tail
    // bit mask.
    newTailMask = 0x0;
    if( remnant ) {
        newTailMask = (Word_t(1) << remnant) - 1;
    }
    // TODO: do not re-allocate block if newSize < _size;
    Word_t * newData = _alloc(newNWords);
    if( !empty() ) {
        memcpy( newData, _data
              , newNWords > _nWords ? _nWords : newNWords );
        _delete(_data);
    }
    _data = newData;
    _size = newSize;
    _nWords = newNWords;
    _tailMask = newTailMask;
}

Bitset &
Bitset::flip(size_t n) {
    assert(n < size());
    _data[n/nBiW] ^= Word_t{1} << n%nBiW;
    return *this;
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

Bitset &
Bitset::flip() {
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] = ~_data[nw];
    }
    return *this;
}

Bitset
Bitset::operator~() const {
    Bitset r(*this);
    return r.flip();
}

Bitset &
Bitset::bitwise_and( const Bitset & bs ) {
    assert( size() == bs.size() );
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] &= bs._data[nw];
    }
    return *this;
}

Bitset
Bitset::operator&( const Bitset & bs ) const {
    Bitset r(*this);
    return r.bitwise_and(bs);
}

Bitset &
Bitset::bitwise_or( const Bitset & bs ) {
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] |= bs._data[nw];
    }
    return *this;
}

Bitset
Bitset::operator|( const Bitset & bs ) const {
    Bitset r(*this);
    return r.bitwise_or(bs);
}

Bitset &
Bitset::bitwise_xor( const Bitset & bs ) {
    for( size_t nw = 0; nw < _nWords; ++nw ) {
        _data[nw] ^= bs._data[nw];
    }
    return *this;
}

Bitset
Bitset::operator^( const Bitset & bs ) const {
    Bitset r(*this);
    return r.bitwise_xor(bs);
}

std::string
Bitset::to_string() const {
    std::string s(size()+1, '\0');
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        for( size_t nb = 0; nb < nBiW; ++nb ) {
            s[size() - (nw*nBiW + nb) - 1] = ((Word_t(1) << nb) & _data[nw] ? '1' : '0' );
        }
    }
    for( size_t nb = 0; nb < _size%nBiW; ++nb ) {
        s[size() - ((_nWords-1)*nBiW + nb) - 1] = ((Word_t(1) << nb) & _data[_nWords-1] ? '1' : '0' );
    }
    return s;
}

template<> std::string
Bitset::to<std::string>() const {
    return to_string();
}

# if 0
void
Bitset::dump( std::ostream & os ) {
    if(empty()) return;
    os << "bitset-" << this << ": ";
    for( size_t nw = 0; nw < _nWords-1; ++nw ) {
        for( int nb = nBiW - 1; nb >= 0 ; --nb ) {
            os << ((Word_t(1) << nb) & _data[nw] ? '1' : '0' );
        }
        os << "(" << std::hex << _data[nw] << ") ";
    }
    for( signed long nb = nBiW - 1; nb >= 0 ; --nb ) {
        if( _size%nBiW > (size_t) nb ) {
            os << ((Word_t(1) << nb) & _data[_nWords-1] ? '1' : '0' );
        } else {
            os << ((Word_t(1) << nb) & _data[_nWords-1] ? 'i' : 'o' );
        }
    }
    os << "(" << std::hex << _data[_nWords - 1] << ")";
    // XXX:
    os << ", _size = " << std::dec << _size
       << ", sizeof(Word_t)*8 = " << nBiW
       << ", _nWords = " << _nWords
       << ", _tailMask = " << std::hex << (int) _tailMask << std::dec << "(" << (int) _tailMask << ")"
       << ", std::string -> " << to_string() << std::endl;
}
# endif

}  // namespace goo

