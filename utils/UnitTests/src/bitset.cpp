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
    //bs.dump(os);
    os << bs;
    os << " [ any = " << (bs.any() ? "true" : "false")
       << ", all = " << (bs.all() ? "true" : "false")
       << ", none = " << (bs.none() ? "true" : "false")
       << ", test(" << nBit << ") = " << (bs.test(nBit) ? "true" : "false")
       << "]" << std::endl
       ;
    _ASSERT(   bs.any(),  "Non-empty set's any() is false (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.all(),  "Non-empty set's all() is true (%zu, %zu).", nBits, nBit );
    _ASSERT( ! bs.none(), "Non-empty set's none() is false (%zu, %zu).", nBits, nBit );
    _ASSERT(   bs.test(nBit), "Bit #%zu was not set (of %zu bits).", nBit, nBits );

    bs.set();
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
    //bs.dump(os);
} GOO_UT_END( Bitset )

