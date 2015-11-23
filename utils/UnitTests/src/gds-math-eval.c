# include "gds/arithmetic_eval.h"
# include "gds-math-eval.h"
# include "goo_types.h"

/* Note: implemented in C++ part */
int binop_key_is_unique( BinOpKey );

static const TypeCode _static_typeCodes[] = {
    # define substitute_code( code, u1, u2, u3 ) code,
    for_all_atomic_datatypes( substitute_code )
    # undef substitute_code
};
static const uint16_t _static_typeCodesSize = sizeof( _static_typeCodes ) / sizeof( TypeCode );

static const BinaryArithOpCode _static_binopCodes[] = {
    # define substitute_code( u1, code, dscr ) code,
    for_each_binary_arithmetic_lexical_operator( substitute_code )
    # undef substitute_code
};
static const uint16_t _static_binopCodesSize = sizeof( _static_binopCodes ) / sizeof( BinaryArithOpCode );

static int
testing_function_summation1(
                struct ArithmeticConstant * l,
                struct ArithmeticConstant * r) {
    return 0;
}

static int
testing_function_summation2(
                struct ArithmeticConstant * l,
                struct ArithmeticConstant * r) {
    return 1;
}

static int
testing_function_production3(
                struct ArithmeticConstant * l,
                struct ArithmeticConstant * r) {
    return 2;
}

int
goo_gds__check_codes_structures() {
    /* Check code translations. */
    # define return_if_not( expr, shift ) if( !(expr) ) { return ((int) rc << sizeof(UByte)) | shift; }
    int rc = 0;
    for( uint16_t i = 0; i < _static_typeCodesSize; ++i ) {
        const TypeCode left = _static_typeCodes[i];
        for( uint16_t j = 0; j < _static_typeCodesSize; ++j ) {
            const TypeCode right = _static_typeCodes[j];
            for( uint16_t k = 0; k < _static_binopCodesSize; ++k, rc+=4 ) {
                BinaryArithOpCode opcode = _static_binopCodes[k];
                {
                    BinOpKey key = gds_compose_binop_key( left, right, opcode );
                    return_if_not( left  == gds_binop_left_oprand_type(  key ), 1 );
                    return_if_not( right == gds_binop_right_oprand_type( key ), 2 );
                    return_if_not( opcode == gds_binop_operator_type( key ), 3 );
                    return_if_not( binop_key_is_unique(key), 4 );
                }
            }
        }
    }
    /* Check table */
    struct GDS_BinOpArithmetic * table = gds_alloc_binop_table();
    BinaryArithOpCode sumCode = GDS_e_binary_summation,
                      prodCode = GDS_e_binary_production
                      ;
    TypeCode t1 = UByte_T_code,
             t2 = Float8_T_code,
             rt = Float8_T_code
             ;
    int insertionResult = gds_add_binary_operator_table_entry(
                    table, sumCode, t1, t2, rt, testing_function_summation1, 0 );
    return_if_not( 0 == insertionResult, 1 );
    insertionResult = gds_add_binary_operator_table_entry(
                    table, sumCode, t1, t2, rt, testing_function_summation1, 0 );
    return_if_not( 1 == insertionResult, 2 );
    insertionResult = gds_add_binary_operator_table_entry(
                    table, sumCode, t1, t2, rt, testing_function_summation1, 1 );
    return_if_not( 1 == insertionResult, 2 );
    insertionResult = gds_add_binary_operator_table_entry(
                    table, sumCode, t1, t2, rt, testing_function_production3, 0 );
    return_if_not( -1 == insertionResult, 3 );
    insertionResult = gds_add_binary_operator_table_entry(
                    table, sumCode, t1, t2, rt, testing_function_summation2, 1 );
    return_if_not( 2 == insertionResult, 4 );
    insertionResult = gds_add_binary_operator_table_entry(
                    table, prodCode, t1, t2, rt, testing_function_production3, 0 );
    return_if_not( 0 == insertionResult, 1 );

    /* TODO: added operating functions tests */

    gds_free_binop_table( table );
    return 0;
}

