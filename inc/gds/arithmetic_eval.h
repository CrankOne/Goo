# ifndef H_GOO_GDS_INTERP_TABLES_H
# define H_GOO_GDS_INTERP_TABLES_H

# include "gds/expression.h"

# ifdef __cplusplus
extern "C" {
# endif

/**\defgroup GDS-simple-arithmetics
 * 
 * @{*/

/**\brief Binary operator key type.*/
typedef uint32_t BinOpKey;

/**\brief Simple arithmetic table structure. */
struct GDS_BinOpArithmetic {
    /** C++ std::unordered_map< TypeCode.TypeCode, pair(resType, SimpleArithmeticBinaryOperator) > */
    void * hashTable;
};

/**\brief Provides search among table of simple arithmetic callbacks. */
SimpleArithmeticBinaryOperator
gds_get_binary_operator_function_for(
        struct GDS_BinOpArithmetic * table,
        BinOpKey,
        TypeCode * );

/**\brief Returns result binary operator result type for given left/right
 * operand types.
 */ TypeCode
gds_earn_binary_arithmetical_operator_result_type(
    struct GDS_BinOpArithmetic * tableStruct,
    BinaryArithOpCode binOpCode,
    TypeCode,
    TypeCode);

/**\brief Tries to figure out returning type of overall expression
 * assuming that all sub-expressions are simple arithmetics.
 */ TypeCode
gds_deduce_arithmetical_expression_result_type(
    struct GDS_BinOpArithmetic *,
    struct GDSExpression *);

/**\brief Recursively resolves binary operator calculating function pointers. */
TypeCode
gds_recache_binary_arithmetical_operator_functions(
    struct GDS_BinOpArithmetic *,
    struct GDSExpression *);

/**\brief (ctr) Allocate simple arithmetics table. */
struct GDS_BinOpArithmetic * gds_alloc_binop_table();
/**\brief (dtr) Free simple arithmetics table. */
void gds_free_binop_table( struct GDS_BinOpArithmetic * );

/**\brief Returns properly-formed binary operator numerical identifier. */
BinOpKey gds_compose_binop_key(TypeCode, TypeCode, BinaryArithOpCode);
/**\brief Obtains left operand type code from operator numerical identifier. */
TypeCode gds_binop_left_oprand_type( BinOpKey k );
/**\brief Obtains right operand type code from operator numerical identifier. */
TypeCode gds_binop_right_oprand_type( BinOpKey k );
/**\brief Obtains arithmetical operator code from full operator numerical identifier. */
BinaryArithOpCode gds_binop_operator_type( BinOpKey k );

/**\brief Inserts evaluator function in simple arithmetics table.
 */ int
gds_add_binary_operator_table_entry(
    struct GDS_BinOpArithmetic * table,
    BinaryArithOpCode binOpCode,
    TypeCode leftT, TypeCode rightT, TypeCode resultT,
    SimpleArithmeticBinaryOperator func,
    uint8_t doOverride );


/**\brief simple arithmetics evaluator.
 */ TypeCode gds_evaluate_binary_arithmetical_operator(
        struct BinaryArithmeticOperator *,
        struct ArithmeticConstant * );

/** @} */

# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_GDS_INTERP_TABLES_H */

