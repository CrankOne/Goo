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

static std::unordered_set<BinOpKey> _static_uniqSet;

extern "C" int
binop_key_is_unique( BinOpKey c ) {
    auto res = _static_uniqSet.insert(c);
    return res.second ? 1 : 0;
}

GOO_UT_BGN( GDS_MATH_EVAL, "GDS simple arithmetic evaluator" )

{
    int rc = goo_gds__check_codes_structures();
    _ASSERT( !rc, "Checking of arithmetic operations subtable code system failed: %d.", rc );
    os << "Code table test passed with rc = " << rc << std::endl;
}

GOO_UT_END( GDS_MATH_EVAL )


