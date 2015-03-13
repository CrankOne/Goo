# ifndef H_GOO_GDS_FUNCTION_H
# define H_GOO_GDS_FUNCTION_H

# include "gds/goo_gds_forwards.h"
# include "goo_config.h"
# include "goo_types.h"

/**@file goo_literals.h
 * @brief File contains general parser state machine routines.
 *
 * Parsed literals represents logic, numerical or string immutable
 * data.
 */

/*
 * Math function
 *
 * All parsed mathematical expressions are presented in
 * intermediate form as a tree-like topological structure.
 * 
 * Each node has a descriptor encoding how this node is
 * to be interpreted further, on evaluation:
 *      node is a local variable            xxxx xx10
 *      node is a numeric value             xxxx xx01
 *      node represents a math operation    xxxx xx11
 *        * Binary:
 *          - summation                     xx00 0011
 *          - subtraction                   xx00 0111
 *          - multiplication                xx00 1011
 *          - division                      xx00 1111
 *          - power (exponentiation)        xx01 0011
 *          - modulo (division remainder)   xx01 0111
 *          - dot (dot product)             xx10 0011
 *        * Unary:
 *          - unary negotiation             xx01 1111
 *  Where bits tagged as 'x' is unused and can be undefined.
 */

struct gds_Function {
    uint8_t descriptor;
    union {
        /* Node is a local variable */
        struct {
            uint8_t orderNum;
        } asLocalVariable;
        /* Node is numeric value */
        struct gds_Literal * asValue;
        /* Node represents mathematical (binary or unary) operation */
        struct {
            struct gds_Function * l, * r;
        } asMathOperation;
    } content;
};

struct gds_Function * gds_math_new_func_from_const(
        struct gds_Parser *,
        struct gds_Literal * );
struct gds_Function * gds_math_new_func_from_locvar(
        struct gds_Parser *,
        struct gds_Literal * );

struct gds_Function * gds_math(
        struct gds_Parser *,
        char opType,
        struct gds_Function *,
        struct gds_Function *);

struct gds_Function * gds_math_negotiate(
        struct gds_Parser *,
        struct gds_Function * );

# endif  /* H_GOO_GDS_FUNCTION_H */

