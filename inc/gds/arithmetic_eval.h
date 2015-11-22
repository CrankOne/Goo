# ifndef H_GOO_GDS_INTERP_TABLES_H
# define H_GOO_GDS_INTERP_TABLES_H

# include "gds/expression.h"

/** This function only evaluates simple binary arithmetics. The point is to
 * resolve corresponding evaluation function that actually does the job. */
int
evaluate_binary_arithmetical_operator(
    BinaryArithOpCode binOpCode,
    struct ArithmeticConstant * const left,
    struct ArithmeticConstant * const right,
    struct ArithmeticConstant result );

/** Returns type code of result considering operator and operand types following
 * the suggestion that simple arithmetic type operation always issues only the
 * simple arithmetic type. This function is useful for strict type checking, but
 * is less generic than TODO(function that returns GDS type code) */
TypeCode
earn_binary_arithmetical_operator_result_type(
    BinaryArithOpCode binOpCode,
    TypeCode,
    TypeCode);


/** Type of binary operator calculator callback function. */
typedef int (*BinaryOperatorFunction)( struct ArithmeticConstant *, struct ArithmeticConstant * );

struct GDS_BinOpArithmetic {
    /* std::unordered_map< TypeCode.TypeCode, pair(resType, BinaryOperatorFunction) > */
    void * hashTable;
};

struct GDS_BinOpArithmetic * gds_alloc_binop_table();
void free_alloc_binop_table( struct GDS_BinOpArithmetic * );

/** Returns:
 * 0 on successful insertion;
 * 1 on reentrant insertion (insertion of the same function instance);
 * -1 on collision (if doOverride == 0, insertion won't be provided).
 */
int
add_binary_operator_table_entry(
    struct GDS_BinOpArithmetic * table,
    BinaryArithOpCode binOpCode,
    TypeCode, TypeCode,
    BinaryOperatorFunction,
    uint8_t doOverride=0 );

# endif  /* H_GOO_GDS_INTERP_TABLES_H */

