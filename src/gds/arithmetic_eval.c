# include "gds/arithmetic_eval.h"

BinOpKey
gds_compose_binop_key( TypeCode leftT, TypeCode rightT, BinaryArithOpCode binOpCode) {
    return leftT
         | ( ((BinOpKey) rightT) << 8*sizeof(TypeCode) )
         | ( ((BinOpKey) binOpCode) << 16*sizeof(TypeCode) )
         ;
}

TypeCode
gds_binop_left_oprand_type( BinOpKey k ) {
    return (0xff & k);
}

TypeCode
gds_binop_right_oprand_type( BinOpKey k ) {
    return ((0xff << 8*sizeof(TypeCode)) & k) >> 8*sizeof(TypeCode);
}

BinaryArithOpCode
gds_binop_operator_type( BinOpKey k ) {
    return ((0xff << 16*sizeof(TypeCode)) & k) >> 16*sizeof(TypeCode);
}

