# include "gds/arithmetic_eval.h"

BinOpKey
gds_compose_binop_key( TypeCode leftT, TypeCode rightT, BinaryArithOpCode binOpCode) {
    return leftT
         | ( ((BinOpKey) rightT) << sizeof(TypeCode) )
         | ( ((BinOpKey) binOpCode) << 2*sizeof(TypeCode) )
         ;
}

TypeCode
gds_binop_left_oprand_type( BinOpKey k ) {
    return (0xff & k);
}

TypeCode
gds_binop_right_oprand_type( BinOpKey k ) {
    return ((0xff << sizeof(TypeCode)) & k) >> sizeof(TypeCode);
}

BinaryArithOpCode
gds_binop_operator_type( BinOpKey k ) {
    return ((0xff << 2*sizeof(TypeCode)) & k) >> 2*sizeof(TypeCode);
}

