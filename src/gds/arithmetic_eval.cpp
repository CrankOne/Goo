# include "gds/arithmetic_eval.h"
# include <unordered_map>

/**\defgroup GDS-simple-arithmetics
 * \ingroup GDS
 *
 * GDS simple arithmetics abilities includes only processing of
 * integral and floating-point numbers. Like in many other languages
 * such expressions will be calculated in a way when, for binary operator,
 * the largest type from two operands will be choosen. In order to provide
 * finer control of math evaluation procedure (which can be desiravle in
 * some rare cases) we exposed internal routines included in this group.
 *  @{
 */

static_assert(sizeof( BinOpKey ) >= 2*sizeof(TypeCode) + sizeof(BinaryArithOpCode),
        "Binary arithmetic indexing type is smaller than need.");

typedef std::unordered_map<BinOpKey, std::pair<TypeCode, SimpleArithmeticBinaryOperator> > BOTable;

/** Allocates internal table of simple arithmetic evaluation functions.
 * Internally invokes heap allocation routines (`malloc()` for C struct
 * \ref GDS_BinOpArithmetic and
 * `new` for `std::unordered_map` container).
 *
 * \see gds_free_binop_table()
 *
 * \returns Newly allocated callback table for simple arithmetics in GDS.
 */ extern "C" struct GDS_BinOpArithmetic *
gds_alloc_binop_table() {
    GDS_BinOpArithmetic * r = (GDS_BinOpArithmetic *) malloc(sizeof(GDS_BinOpArithmetic));
    r->hashTable = new BOTable();
    return r;
}

# define LCL_TABLE(a, b) BOTable * b = reinterpret_cast<BOTable*>(a->hashTable)

/** Frees heap-allocated table for simple arithmetics in GDS.
 * Internally calls appropriate heap deallocation routines.
 *
 * \see gds_alloc_binop_table()
 *
 * \param tableStruct previously allocated with \ref gds_alloc_binop_table() table
 * for simple arithmetics in GDS.
 */ extern "C" void
gds_free_binop_table( struct GDS_BinOpArithmetic * tableStruct ) {
    LCL_TABLE( tableStruct, table );
    delete table;
    tableStruct->hashTable = NULL;
    free( tableStruct );
}

/**This function handles only for simple arithmetics evaluator table.
 *
 * \param tableStruct a simple arithmetics evaluator function table.
 * \param binOpCode binary operator code according
 * to \ref for_each_common_binary_arithmetic_lexical_operator macro.
 * \param leftT left operand type code
 * \param rightT right operand type code
 * \param resultT expected result type code
 * \param func evaluator function
 * \param doOverride defines behaivour in case of colliding insertion
 * (see return results).
 * \returns 0 on successful insertion;
 * \returns 1 on reentrant insertion (insertion of the same function instance);
 * \returns 2 on override (only available on doOverride).
 * \returns -1 on collision (if doOverride == 0, insertion won't be provided).
 * \returns -3 indicates algorithm error.
 */ extern "C" int
gds_add_binary_operator_table_entry(
        struct GDS_BinOpArithmetic * tableStruct,
        BinaryArithOpCode binOpCode,
        TypeCode leftT, TypeCode rightT, TypeCode resultT,
        SimpleArithmeticBinaryOperator func,
        uint8_t doOverride ) {
    LCL_TABLE( tableStruct, table );
    bool doRepeatInsertion = false,
         overriden = false;
    do {
        doRepeatInsertion = false;
        auto insertionResult = table->insert(
                std::pair< BinOpKey, std::pair<TypeCode, SimpleArithmeticBinaryOperator> >(
                    gds_compose_binop_key( leftT, rightT, binOpCode ),
                    std::pair<TypeCode, SimpleArithmeticBinaryOperator>( resultT, func )
                )
            );
        if( !insertionResult.second ) {
            if( insertionResult.first->second.second == func ) {
                return 1;
            }
            if( doOverride ) {
                doRepeatInsertion = true;
                overriden = true;
                table->erase( insertionResult.first );
            } else {
                return -1;
            }
        } else {
            return overriden ? 2 : 0;
        }
    } while(doRepeatInsertion);
    return -3;
}

/** Provides lookup below simple operators functions listed in table using unique
 * identifier. If resultTypePtr is NULL, than it wouldn't be written.
 * \param tableStruct a simple arithmetic table instance.
 * \param key an unique operator identificator
 * \param resultTypePtr a pointer where to write expected result type.
 * \returns Evaluating function.
 */ SimpleArithmeticBinaryOperator
gds_get_binary_operator_function_for(
        struct GDS_BinOpArithmetic * tableStruct,
        BinOpKey key,
        TypeCode * resultTypePtr ) {
    LCL_TABLE( tableStruct, table );
    auto it = table->find( key );
    if( table->end() == it ) {
        return NULL;
    }
    if( resultTypePtr ) {
        *resultTypePtr = it->second.first;
    }
    return it->second.second;
}

/**\param tableStruct a simple arithmetics evaluator function table.
 * \param binOpCode binary operator code according
 * to \ref for_each_common_binary_arithmetic_lexical_operator macro.
 * \param leftT left operand type code
 * \param rightT right operand type code
 * \returns type code of result considering operator and operand types following
 * the suggestion that simple arithmetic type operation always issues only the
 * simple arithmetic type. This function is useful for strict type checking, but
 * is less generic than (TODO:ref to function that returns GDS type code).
 * \returns «0» if result type can not be figured out within current table state,
 * or at least one of binOpCode/leftT/rightT arguments is set to 0.
 */ extern "C" TypeCode
gds_earn_binary_arithmetical_operator_result_type(
    struct GDS_BinOpArithmetic * tableStruct,
    BinaryArithOpCode binOpCode,
    TypeCode leftT,
    TypeCode rightT) {
    if(!(binOpCode && leftT && rightT)) {
        return 0x0;
    }
    LCL_TABLE( tableStruct, table );
    auto it = table->find( gds_compose_binop_key( leftT, rightT, binOpCode ) );
    if( table->end() == it ) {
        return 0x0;
    }
    return it->second.first;
}

/** @} */

