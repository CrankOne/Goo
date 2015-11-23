# include <cstring>
# include <set>
# include "utest.hpp"
# include "gds-math-eval.h"
# include "gds/arithmetic_eval.h"

# include <iostream>

/**@file gds-math-eval.cpp
 * @brief GDS interpretator base elvel test.
 *
 * This file provides testing of some auxilliary basics in GDS evaluator.
 * */

static std::unordered_set<BinaryArithOpCode> _static_uniqSet;

extern "C" int
binop_key_is_unique( BinaryArithOpCode c ) {
    auto res = _static_uniqSet.insert(c);
    return res.second;
}

GOO_UT_BGN( GDS_MATH_EVAL, "GDS simple arithmetic evaluator" )

{
    int rc = goo_gds__check_codes_structures();
    os << "Code table test passed with rc = " << rc << std::endl;
    _ASSERT( !rc, "Checking of arithmetic operations subtable code system failed: %d.", rc );
}

GOO_UT_END( GDS_MATH_EVAL )


