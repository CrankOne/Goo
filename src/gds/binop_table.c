# include <assert.h>
# include <math.h>
# include "gds/expression.h"

/*
 *  This file implements binary operators tabular functions for atomic
 * arithmetical types like n-byte integer and floating-point numbers.
 * For now, there is only one function per operation ...
 */

# define do_summation_with( a, b )      ( (a) + (b) )
# define do_subtraction_with( a, b )    ( (a) - (b) )
# define do_production_with( a, b )     ( (a) * (b) )
# define do_division_with( a, b )       ( (a) / (b) )
# define do_exponentiation_with( a, b ) (pow( a, b ))
# define do_divRemainder_with( a, b )   ( (a) % (b) )

static TypeCode
get_longest_type( TypeCode a, TypeCode b ) {
    if( get_atomic_type_size(a) > get_atomic_type_size(b)
        || a == b ) {
        return a;
    } else if( get_atomic_type_size(a) == get_atomic_type_size(b) ) {
        if( a == Float4_T_code || b == Float4_T_code ) {
            return Float4_T_code;
        } else if( a == Float8_T_code || b == Float8_T_code ) {
            return Float8_T_code;
        }
        # ifdef TYPES_128BIT_LENGTH
        else if( a == Float16_T_code || b == Float16_T_code ) {
            return Float16_T_code;
        }
        # endif
        else {
            goo_C_error( goo_e_narrowConversion,
                    "Couldn't determine highest type for %x and %x.",
                    a, b);
        }
    } else {
        return b;
    }
}

# define EMPTY(...)
# define DEFER(...) __VA_ARGS__ EMPTY()
# define EXPAND(...) __VA_ARGS__

# define implement_operation( opname, lType, rType )                    \
TypeCode                                                                \
opname ## _ ## lType ## _ ## rType (                                    \
                struct ArithmeticConstant * l,  /* any */               \
                struct ArithmeticConstant * r,  /* any */               \
                struct ArithmeticConstant * R   /* largest */ ) {       \
    assert( l );                                                        \
    assert( r );                                                        \
    assert( R );                                                        \
    R->value.Float4Val = do_ ## opname ## _with(                        \
            (l->value. lType ## Val),                                   \
            (r->value. rType ## Val)                                    \
        );                                                              \
    return R->type = get_longest_type( l->type, r->type );              \
}

# define for_all_atomic_datatypes_def()             for_all_atomic_datatypes

# define mmm3( x1, x2, rtype, x3, ltype, opname )   implement_operation( opname, ltype, rtype )
# define mmm2( x1, x2, ltype, x3, opname )          DEFER(for_all_atomic_datatypes_def)()( mmm3, ltype, opname )
# define mmm1( x1, x2, opname )                     EXPAND(for_all_atomic_datatypes( mmm2, opname ))

for_each_binary_arithmetic_lexical_operator( mmm1 )

# undef mmm1
# undef mmm2
# undef mmm3

# undef implement_operation

