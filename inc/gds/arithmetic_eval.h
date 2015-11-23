# ifndef H_GOO_GDS_INTERP_TABLES_H
# define H_GOO_GDS_INTERP_TABLES_H

# include "gds/expression.h"

/** This function only evaluates simple binary arithmetics. The point is to
 * resolve corresponding evaluation function that actually does the job. */
int
gds_evaluate_binary_arithmetical_operator(
    BinaryArithOpCode binOpCode,
    struct ArithmeticConstant * const left,
    struct ArithmeticConstant * const right,
    struct ArithmeticConstant result );

/** Returns type code of result considering operator and operand types following
 * the suggestion that simple arithmetic type operation always issues only the
 * simple arithmetic type. This function is useful for strict type checking, but
 * is less generic than TODO(function that returns GDS type code) */
TypeCode
gds_earn_binary_arithmetical_operator_result_type(
    BinaryArithOpCode binOpCode,
    TypeCode,
    TypeCode);


typedef uint32_t BinOpKey;

/** Type of binary operator calculator callback function. */
typedef int (*BinaryOperatorFunction)( struct ArithmeticConstant *, struct ArithmeticConstant * );

struct GDS_BinOpArithmetic {
    /* std::unordered_map< TypeCode.TypeCode, pair(resType, BinaryOperatorFunction) > */
    void * hashTable;
};

struct GDS_BinOpArithmetic * gds_alloc_binop_table();
void gds_free_alloc_binop_table( struct GDS_BinOpArithmetic * );

# ifdef __cplusplus
extern "C" {
# endif
BinOpKey gds_compose_binop_key(TypeCode, TypeCode, BinaryArithOpCode);
TypeCode gds_binop_left_oprand_type( BinOpKey k );
TypeCode gds_binop_right_oprand_type( BinOpKey k );
BinaryArithOpCode gds_binop_operator_type( BinOpKey k );
# ifdef __cplusplus
}
# endif

/** Returns:
 * 0 on successful insertion;
 * 1 on reentrant insertion (insertion of the same function instance);
 * -1 on collision (if doOverride == 0, insertion won't be provided).
 */
int
gds_add_binary_operator_table_entry(
    struct GDS_BinOpArithmetic * table,
    BinaryArithOpCode binOpCode,
    TypeCode, TypeCode,
    BinaryOperatorFunction,
    uint8_t doOverride );

# endif  /* H_GOO_GDS_INTERP_TABLES_H */

