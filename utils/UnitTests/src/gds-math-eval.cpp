# include <cstring>
# include <set>
# include <cstddef>
# include "utest.hpp"
# include "gds-math-eval.h"
# include "gds/arithmetic_eval.h"

# include <iostream>

/**@file gds-math-eval.cpp
 * @brief GDS interpretator base elvel test.
 *
 * This file provides testing of some auxilliary basics in GDS evaluator.
 * */

static std::unordered_set<BinOpKey> _static_uniqSet;

extern "C" int
binop_key_is_unique( BinOpKey c ) {
    auto res = _static_uniqSet.insert(c);
    return res.second ? 1 : 0;
}

GOO_UT_BGN( GDS_MATH_EVAL, "GDS simple arithmetic evaluator" )

{
    {
        struct DataOffsets {
                const char * name;
                size_t offset, size; } arithOffsets[] = {
            # define set_offset_entry_table( code, cTypeName, gooName, gdsName )    \
                { # cTypeName,                                                      \
                  offsetof( ArithmeticConstant::ArithmeticValue, gooName ## Val ),  \
                  sizeof( cTypeName ) } ,
            for_all_atomic_datatypes( set_offset_entry_table )
            # undef set_offset_entry_table
        };
        char * bf;
        size_t bflen;
        FILE * of = ::open_memstream( &bf, &bflen );
        fprintf( of, "In whole %zu bytes:\n", sizeof(ArithmeticConstant::ArithmeticValue) );
        for( struct DataOffsets & entry : arithOffsets ) {
            fprintf(of, " %-10s  %3zu %3zu\n", entry.name, entry.offset, entry.size );
        }
        ::fclose(of);
        os << bf;
        free( bf );
    }
    

    int rc = goo_gds__check_codes_structures();
    _ASSERT( !rc, "Checking of arithmetic operations subtable code system failed #1: %d.", rc );
    os << "Code table test passed with rc = " << rc << std::endl;
    rc = goo_gds__check_simple_arithmetics_evaluation();
    _ASSERT( !rc, "Checking of arithmetic operations subtable code system failed #1: %d.", rc );
    os << "Simple evaluation test passed with rc = " << rc << std::endl;
}

GOO_UT_END( GDS_MATH_EVAL )


