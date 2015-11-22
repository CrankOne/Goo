# include "gds/arithmetic_eval.h"

# if 0
# define deduced_type_entry( m, largestType, type ) \
    m( largestType,     largestType,    type )      \
    m( largestType,     type,    largestType )

# define for_all_binop_combination_int8( m )        \
    m(               Int8,  Int8,   Int8    )       \
    deduced_type_entry( m,  Int8,   UInt8   )       \
    deduced_type_entry( m,  Int16,  Int8    )       \

# define for_all_binop_combination_uint8(m)\
    m( UInt8,       UInt8,      UInt8   ) \
    m( UInt16,      UInt16,     UInt8   ) \
    m( UInt16,      UInt8,      UInt16  ) \
    m( UInt32,      UInt32,     UInt8   ) \
    m( UInt32,      UInt8,      UInt32  ) \
    m( UInt64,      UInt64,     UInt8   ) \
    m( UInt64,      UInt8,      UInt64  ) \
    m( Float32,     Float32,    UInt8   ) \
    m( Float32,     UInt8,      Float32 ) \
    m( Float64,     Float64,    UInt8   ) \
    m( Float64,     Uint8,      Float64 ) \
    /* ... */

int evaluate_binary_arithmetical_operator(
    BinaryArithOpCode binOpCode,
    struct ArithmeticConstant * const l,
    struct ArithmeticConstant * const r,
    struct ArithmeticConstant * R ) {

    # if 0
    R->value. resType ## Val = func ( l->value. lType ## Val, r->value. rType ## Val )
    # endif
}
# endif

