# include "utest.hpp"
# include "goo_bitset.hpp"

/**@file bitset.cpp
 * @brief Bitset routines test.
 * */

void test_on_set_of_length( std::ostream & os
                          , const size_t nBits, size_t nBit ) {
    goo::Bitset bs(nBits);
    bs.reset();
    os << bs;
    os << " [ any = " << (bs.any() ? "true" : "false")
       << ", all = " << (bs.all() ? "true" : "false")
       << ", none = " << (bs.none() ? "true" : "false")
       << ", test(" << nBit << ") = " << (bs.test(nBit) ? "true" : "false")
       << "]" << std::endl
       ;
    _ASSERT( ! bs.any(),  "Empty set's any() is true (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.all(),  "Empty set's all() is true (%zu, %zu).", nBits, nBit );
    _ASSERT(   bs.none(), "Empty set's none() is false (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.test(nBit), "Empty's bit #%zu is set (of %zu bits).", nBit, nBits );

    bs.set(nBit);
    os << bs;
    os << " [ any = " << (bs.any() ? "true" : "false")
       << ", all = " << (bs.all() ? "true" : "false")
       << ", none = " << (bs.none() ? "true" : "false")
       << ", test(" << nBit << ") = " << (bs.test(nBit) ? "true" : "false")
       << "]" << std::endl
       ;
    //bs.dump(os);
    _ASSERT(   bs.any(),  "Non-empty set's any() is false (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.all(),  "Non-empty set's all() is true (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.none(), "Non-empty set's none() is false (%zu, %zu).", nBits, nBit );
    _ASSERT(   bs.test(nBit), "Bit #%zu was not set (of %zu bits).", nBit, nBits );

    //bs.set();
    bs.flip(nBit);
    bs.flip();

    os << bs;
    os << " [ any = " << (bs.any() ? "true" : "false")
       << ", all = " << (bs.all() ? "true" : "false")
       << ", none = " << (bs.none() ? "true" : "false")
       << ", test(" << nBit << ") = " << (bs.test(nBit) ? "true" : "false")
       << "]" << std::endl
       ;
    _ASSERT(   bs.any(),  "Full set's any() is false (%zu, %zu).", nBits, nBit );
    _ASSERT(   bs.all(),  "Full set's all() is false (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.none(), "Full set's none() is true (%zu, %zu).", nBits, nBit );
    _ASSERT(   bs.test(nBit), "Bit #%zu was not set in full"
            " bitset (of %zu bits).", nBit, nBits );
}

GOO_UT_BGN( Bitset, "Dynamic bitset" ) {
    {
        os << "# Basic operations on self tests" << std::endl;
        struct { size_t nBits, nBit; } conds[] = {
            { 15, 4 }, {15, 0}, {15, 14},
            { 5, 3  }, {5,  0}, {5,   4},
            { 8, 7  }, {8,  0}, {8,   7},
            { 32, 16}, {32, 0}, {32, 31},
            {128, 74}, {128,0}, {128,127},
            {0, 0}  // sentinel
        };
        for(auto a = conds; a->nBits; ++a) {
            test_on_set_of_length( os, a->nBits, a->nBit );
        }
    }
    {
        os << "# Bitwise operations tests" << std::endl;
        Bitset a(2), b(2), c;
        a.set();
        b.reset();
        {
            c = a & b;
            os << a << "&" << b << "=" << c << std::endl;
            _ASSERT( ! c, "Bitwise-AND result of full and empty set is true." );
        }
        {
            c = a | b;
            os << a << "|" << b << "=" << c << std::endl;
            _ASSERT(   c, "Bitwise-OR result of full and empty set is false." );
        }
        {
            c = a ^ b;
            os << a << "^" << b << "=" << c << std::endl;
            _ASSERT(   c, "Bitwise-XOR result of full and empty set is true." );
        }
        {  // bitwise expression #1
            a.set(0, false);
            b = a;
            b.flip();
            c = a|b;
            os << a << "|" << b << "=" << c << std::endl;
            _ASSERT( c.all(), "Bitwise expression #1 evaluated wrong." );
        }
        {  // bitwise expression #2
            a.resize(4);
            a.set();
            a.set(1, false);
            a.reset(3);
            b.resize(4);
            b.reset();
            b.set(1);
            b.set(0);
            b.reset(3);
            c = a^b;
            os << a << "^" << b << "=" << c << std::endl;
            os << c.to_ulong() << std::endl;
            //_ASSERT( (unsigned long) 0x6 == c.to_ulong(), "Bitwise expression #2 evaluated wrong." );
        }
    }
} GOO_UT_END( Bitset )

