# include "gds/arithmetic_eval.h"
# include <assert.h>

BinOpKey
gds_compose_binop_key( TypeCode leftT, TypeCode rightT, BinaryArithOpCode binOpCode) {
    return   ((BinOpKey) leftT)
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

/**\returns deduced type code if all sub-expressions is
 * simple binary operators with simple arithmetic types,
 * or if expression is simple arithmetic value itself.
 * \return 0 if expr is NULL, or if expression is neither
 * a simple binary arithmetic operator nor simple arithmetic
 * value.
 */ TypeCode
gds_deduce_arithmetical_expression_result_type(
    struct GDS_BinOpArithmetic * table,
    struct GDSExpression * expr ) {
    if( !expr ) return 0;
    if( gdsE_binArithOperator == expr->expressionType ) {
        return gds_earn_binary_arithmetical_operator_result_type(
                table,
                expr->pointer.binop->type,
                gds_deduce_arithmetical_expression_result_type(
                            table, expr->pointer.binop->left ),
                gds_deduce_arithmetical_expression_result_type(
                            table, expr->pointer.binop->right )
            );
    } else if( gdsE_arithmConstant == expr->expressionType ) {
        return expr->pointer.arithmetic->type;
    } else {
        return 0x0;
    }
}

/**\returns resulting arithmetic type code
 * \returns 0 if if provided expression is NULL, or
 * if expression is not a simple binary arithmetic operator.
 */ TypeCode
gds_recache_binary_arithmetical_operator_functions(
    struct GDS_BinOpArithmetic * table,
    struct GDSExpression * expr ) {
    if( !expr ) {
        return 0;
    }
    if( gdsE_arithmConstant == expr->expressionType ) {
        return expr->pointer.arithmetic->type;
    } else if( gdsE_binArithOperator != expr->expressionType ) {
        return 0;
    }
    TypeCode resultT;
    expr->pointer.binop->resolvedFunctionCache.simple_arithmetic_binary_operator =
        gds_get_binary_operator_function_for( 
            table,
            gds_compose_binop_key(
                    gds_recache_binary_arithmetical_operator_functions(
                        table, expr->pointer.binop->left ),
                    gds_recache_binary_arithmetical_operator_functions(
                        table, expr->pointer.binop->right ),
                    expr->pointer.binop->type
            ),
            &resultT
        );
    return resultT;
}


TypeCode
gds_evaluate_binary_arithmetical_operator(
        struct BinaryArithmeticOperator * op,
        struct ArithmeticConstant * res ) {
    assert( op->resolvedFunctionCache.f );
    struct ArithmeticConstant l, r;
    if( gdsE_binArithOperator == op->left->expressionType ) {
        gds_evaluate_binary_arithmetical_operator( op->left->pointer.binop, &l );
    } else if( gdsE_arithmConstant == op->left->expressionType ) {
        l = *(op->left->pointer.arithmetic);
    } else {
        return 0;
    }
    if( gdsE_binArithOperator == op->right->expressionType ) {
        gds_evaluate_binary_arithmetical_operator( op->right->pointer.binop , &r );
    } else if( gdsE_arithmConstant == op->right->expressionType ) {
        r = *(op->right->pointer.arithmetic);
    } else {
        return 0;
    }
    return op->resolvedFunctionCache.simple_arithmetic_binary_operator( &l, &r, res );
}

