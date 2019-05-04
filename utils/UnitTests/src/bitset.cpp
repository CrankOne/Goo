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
        {
            // stl bitset<N> behaves like:
            // 0	00000	11111	31
            // 1	00001	11110	30
            // 2	00010	11101	29
            // 3	00011	11100	28
            // 4	00100	11011	27
            // 5	00101	11010	26
            // 6	00110	11001	25
            // 7	00111	11000	24
            // 8	01000	10111	23
            // 9	01001	10110	22
            // See e.g.: https://ru.cppreference.com/w/cpp/utility/bitset/to_ulong
            for( unsigned long i = 0; i < 10; ++i ) {
                Bitset b(5, i);
                Bitset b_inverted = ~b;
                os << i << '\t';
                os << b << '\t';
                os << b_inverted << '\t';
                os << b_inverted.to_ulong() << '\n'; 
            }
        }
        {  // bitwise expression #1
            a.set(0, false);
            b = a;
            b.flip();
            c = a|b;
            os << a << "|" << b << "=" << c << std::endl;
            _ASSERT( c.all(), "Bitwise expression #1 gives wrong result." );
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
            //c.dump(os);
            _ASSERT( (unsigned long) 0x6 == c.to_ulong()
                   , "Bitwise expression #2 gives wrong result (ulong): %d != %lu."
                   , 0x6, c.to_ulong() );
            _ASSERT( (unsigned char) 0x6 == c.to<unsigned char>()
                   , "Bitwise expression #2 gives wrong result (uchar)." );
        }
        {  // bitwise expression #3
            goo::Bitset bs(sizeof(unsigned long)*8 - 2);
            unsigned long ctrl = 0;
            const char nBits[] = { 0, 3, 12, 17, 21, 27, -1 };
            for( const char * c = nBits; -1 != *c; ++c ) {
                ctrl |= (1 << *c);
                bs.set( *c );
            }
            os << bs << " = " << std::hex << bs.to_ulong()
               << ", ctrl = " << std::hex << ctrl << std::dec << std::endl;
            //bs.dump(os);
            _ASSERT( ctrl == bs.to_ulong()
                   , "Bitwise expression #3 evaluated wrong (ulong"
                   " control): %lu != %lu.", ctrl, bs.to_ulong() );
        }
    }
} GOO_UT_END( Bitset )

