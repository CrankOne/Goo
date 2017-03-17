# include "gds/expression.h"
# include <stdlib.h>

/**\defgroup GDS-expression
 * \ingroup GDS
 *
 * Expression structure instance is the product of GDS sematical analysis.
 * The single structure \ref GDSExpression combines all possible subtypes
 * of GDS entities: constants, variables, mathematical expressions, scopes,
 * lists, user-defined types, etc. All of basic subtypes of expression are
 * listed in \ref for_each_gds_expr_node_type x-macro. User-defined
 * extensions are provided in next level of reflexivity (TODO: ref to user
 * subtypes).
 *
 * Most of existing expressions are presented as acyclic oriented graphs with
 * arithmetical constants or unresolved symbols as a nodes. Expressions can
 * refer to each other via symbol resolution mechanism (TODO: ref).
 *
 * Processing expression means iterate via this tree with special tree-reduction
 * routines while it remains possible.
 */

/*
 * Arithmetic constant ********************************************************
 */

/** \note the tNode->pointer.arithmetic needs to be allocated!
 *
 *  Return pointer is for convinience --- one can build expressions
 *  from C in one line.
 *
 *  \returns tNode pointer (the same as argument).
 *  \returns NULL, if typeCode doesn't corresponds to any
 *  arithmetical value.
 *  \param typeCode a type code of stored value
 *  \param tNode a target node (to be initialized)
 *  \param value a value instance to be set. */
struct GDSExpression *
gds_init_arithmetic_value_expr(
                TypeCode typeCode,
                struct GDSExpression * tNode,
                union ArithmeticValue value) {
    if( IS_COMPOUND_TYPE(typeCode) ) {
        return NULL;
    }
    tNode->expressionType = gdsE_arithmConstant;
    tNode->pointer.arithmetic->type = typeCode;
    tNode->pointer.arithmetic->value = value;
    return tNode;
}

/*
 * Binary operators ***********************************************************
 */

# define LCL_define_binary_arithmetical_code( sign, code, name ) \
const BinaryArithOpCode GDS_e_binary_ ## name = code;
for_each_common_binary_arithmetic_lexical_operator( LCL_define_binary_arithmetical_code )
# undef LCL_define_binary_arithmetical_code

/** \note the tNode->pointer.binop needs to be allocated!
 *
 *  Return pointer is for convinience --- one can build expressions
 *  from C in one line.
 *
 *  \returns tNode pointer (the same as argument).
 *  \returns NULL if at least one of the tNode/left/right
 *  nodes pointers is NULL.
 *  \param binopCode a code of binary operator
 *  \param left left operand of operator
 *  \param right right operand of operator. */
struct GDSExpression *
gds_init_binary_operator_expr(
                BinaryArithOpCode binopCode,
                struct GDSExpression * tNode,
                struct GDSExpression * left,
                struct GDSExpression * right) {
    if(!(tNode && left && right && binopCode)){
        return NULL;
    }
    tNode->expressionType = gdsE_binArithOperator;
    tNode->pointer.binop->type = binopCode;
    tNode->pointer.binop->resolvedFunctionCache.f = NULL;
    tNode->pointer.binop->left = left;
    tNode->pointer.binop->right = right;
    return tNode;
}


