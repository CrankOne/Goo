# include "gds/arithmetic_eval.h"
# include "gds-math-eval.h"
# include "goo_types.h"

# include <string.h>
# include <stdlib.h>
# include <assert.h>
# include <stdio.h>  // XXX

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
    for_each_common_binary_arithmetic_lexical_operator( substitute_code )
    # undef substitute_code
};
static const uint16_t _static_binopCodesSize = sizeof( _static_binopCodes ) / sizeof( BinaryArithOpCode );

static TypeCode
testing_function_summation1(
                struct ArithmeticConstant * l,  /* UInt */
                struct ArithmeticConstant * r,  /* Float4 */
                struct ArithmeticConstant * R   /* Float4 */ ) {
    assert( l );
    assert( r );
    assert( R );
    R->value.Float4Val = (l->value.UIntVal) + (r->value.Float4Val);
    return R->type = Float4_T_code;
}

static TypeCode
testing_function_summation2(
                struct ArithmeticConstant * l,
                struct ArithmeticConstant * r,
                struct ArithmeticConstant * R) {
    /* function to never be really invoked */
    return 0;
}

static TypeCode
testing_function_production3(
                struct ArithmeticConstant * l,  /* UByte */
                struct ArithmeticConstant * r,  /* Float4 */
                struct ArithmeticConstant * R   /* Float4 */ ) {
    assert( l );
    assert( r );
    assert( R );
    R->value.Float4Val = (l->value.UByteVal)*(r->value.Float4Val);
    return R->type = Float4_T_code;
}

int
goo_gds__check_codes_structures() {
    /* Check code translations. */
    # define return_if_not( expr, shift ) if( !(expr) ) { return ((int) rc << sizeof(UByte)) | shift; }
    int rc = 0, insertionResult = 0;

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
    insertionResult = gds_add_binary_operator_table_entry(
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

    gds_free_binop_table( table );
    return 0;
    # undef return_if_not
}

int
goo_gds__check_simple_arithmetics_evaluation() {
    int insertionResult = 0;  /* todo: change assert()'s w representative returns */
    struct GDS_BinOpArithmetic * table = gds_alloc_binop_table();

    /* Imitates ready operator table */
    printf( " XXX UInt_T_code <- %#x\n", UInt_T_code );  // XXX
    printf( " XXX Float4_T_code <- %#x\n", Float4_T_code );  // XXX
    if( 0 != (insertionResult = gds_add_binary_operator_table_entry(
            table,
            GDS_e_binary_summation,
            UInt_T_code, Float4_T_code, Float4_T_code,
            testing_function_summation1, 0 ))) {
        return -1;
    }
    if( 0 != (insertionResult = gds_add_binary_operator_table_entry(
            table,
            GDS_e_binary_production,
            UByte_T_code, Float4_T_code, Float4_T_code,
            testing_function_production3, 0 ))) {
        return -2;
    }

    /* Now test arithmetics evaluation:
     * 10.02 == 3*(1 + 2.34)
     *
     *   (*) [=> be deduced to Float4]
     *    | - 3 : UByte
     *    \ - (+) [=> be deduced to Float4]
     *         |- 1 : UInt32
     *         \- 2.34 : Float4
     * */
    {
        /* CONSTRUCTION */
        struct GDSExpression nodes[5],
            * multiplyNode    = nodes,
            * uint8ThreeNode  = nodes + 1,
            * sumNode         = nodes + 2,
            * uint32Node      = nodes + 3,
            * float4Node      = nodes + 4
            ;
        bzero( nodes, sizeof(nodes) );
        union ArithmeticValue val1 = { .UByteVal = 3 },
                              val2 = { .UIntVal = 1 },
                              val3 = { .Float4Val = 2.34 }
                              ;
        /* Allocate nodes from heap */
        {
               multiplyNode->pointer.binop  = (struct BinaryArithmeticOperator* ) malloc( sizeof(struct BinaryArithmeticOperator) );
                    sumNode->pointer.binop  = (struct BinaryArithmeticOperator* ) malloc( sizeof(struct BinaryArithmeticOperator) );
        uint8ThreeNode->pointer.arithmetic  = (struct ArithmeticConstant * ) malloc( sizeof(struct ArithmeticConstant) );
            uint32Node->pointer.arithmetic  = (struct ArithmeticConstant * ) malloc( sizeof(struct ArithmeticConstant) );
            float4Node->pointer.arithmetic  = (struct ArithmeticConstant * ) malloc( sizeof(struct ArithmeticConstant) );
        }
        /* Construct expression tree */
        assert( multiplyNode == gds_init_binary_operator_expr(
                GDS_e_binary_production, multiplyNode,
                gds_init_arithmetic_value_expr( UByte_T_code, uint8ThreeNode, val1 ),
                gds_init_binary_operator_expr(
                        GDS_e_binary_summation, sumNode,
                        gds_init_arithmetic_value_expr( UInt_T_code, uint32Node, val2 ),
                        gds_init_arithmetic_value_expr( Float4_T_code, float4Node, val3 )
                )
        ));
        /* CHECKS */
        /* Check type deduction from basic nodes */
        TypeCode foundType;
        if( 0 != (foundType=gds_earn_binary_arithmetical_operator_result_type(
                    table, GDS_e_binary_production,
                    UInt_T_code, Float4_T_code )) ) {
            /* False positive look-up result */
            return -3;
        }
        if( Float4_T_code != (foundType=gds_earn_binary_arithmetical_operator_result_type(
                    table, GDS_e_binary_summation,
                    UInt_T_code, Float4_T_code )) ) {
            /* False negative look-up result */
            return -4;
        }
        if( Float4_T_code != (foundType=gds_earn_binary_arithmetical_operator_result_type(
                    table, GDS_e_binary_production,
                    UByte_T_code, Float4_T_code )) ) {
            return -5;
        }
        /* Check type deduction from simple arithmetic expression */
        TypeCode deducedType;
        if( UInt_T_code != (deducedType = gds_deduce_arithmetical_expression_result_type(
                    table, uint32Node )) ) {
            return -11;
        }
        if( Float4_T_code != gds_deduce_arithmetical_expression_result_type(
                    table, float4Node ) ) {
            return -12;
        }
        if( UByte_T_code != gds_deduce_arithmetical_expression_result_type(
                    table, uint8ThreeNode ) ) {
            return -13;
        }
        if( Float4_T_code != gds_deduce_arithmetical_expression_result_type(
                    table, sumNode ) ) {
            return -14;
        }
        if( Float4_T_code != gds_deduce_arithmetical_expression_result_type(
                    table, multiplyNode ) ) {
            return -15;
        }
        /* Check operator function deduction */
        if( testing_function_summation1 != gds_get_binary_operator_function_for(
                    table,
                    gds_compose_binop_key( UInt_T_code, Float4_T_code, GDS_e_binary_summation ),
                    NULL ) ) {
            return -21;
        }
        if( testing_function_production3 != gds_get_binary_operator_function_for(
                    table,
                    gds_compose_binop_key( UByte_T_code, Float4_T_code, GDS_e_binary_production ),
                    NULL ) ) {
            return -22;
        }
        /* EVALUATION */
        /* Set evaluator caches */
        if( Float4_T_code != gds_recache_binary_arithmetical_operator_functions(table, multiplyNode) ) {
            return -32;
        }
        /* Check evaluation result (type and value; the former --- with threshold) */
        struct ArithmeticConstant res;
        if( Float4_T_code != gds_evaluate_binary_arithmetical_operator(
                    multiplyNode->pointer.binop, &res ) ){
            return -41;
        }
        if( res.value.Float4Val < 10.02 - 1e-4 || res.value.Float4Val > 10.02 + 1e-4 ) {
            return -42;
        }

        /* Free stuff */
        free( multiplyNode->pointer.binop );
        free( uint8ThreeNode->pointer.arithmetic );
        free( sumNode->pointer.binop );
        free( uint32Node->pointer.arithmetic );
        free( float4Node->pointer.arithmetic );
    }
    gds_free_binop_table( table );
    return 0;
}

