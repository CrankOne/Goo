# ifndef H_GOO_GDS_FUNCTION_H
# define H_GOO_GDS_FUNCTION_H

# include "gds/goo_gds_forwards.h"
# include "goo_config.h"
# include "goo_types.h"

# ifdef ENABLE_GDS

/**@file goo_literals.h
 * @brief File contains general parser state machine routines.
 *
 * Parsed literals represents logic, numerical or string immutable
 * data.
 */

/**@struct gds_ArgList
 * @brief Describes a sequence of unknown names.
 *
 * Is used in function declarations.
 * TODO: use hashtable for quicker search.
 */
struct gds_ArgList {
    const char * identifier;
    struct gds_ArgList * next;
};

/** Returns new void arglist entry. */
struct gds_ArgList * gds_math_new_arglist_entry(
        struct gds_Parser * );
/** Sets name of provided arglist entry. */
struct gds_ArgList * gds_math_set_arglist_name(
        struct gds_Parser *,
        struct gds_ArgList *,
        const char *);
/** Appends arglist with new arglist entry */
struct gds_ArgList * gds_math_append_arglist(
        struct gds_Parser *,
        struct gds_ArgList *,
        struct gds_ArgList *);

/**@struct gds_Function
 * @brief Mathematical function description.
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
 *      node is a GDS-math function         1100 0000
 *      node is a third-party math function 1000 0000
 *  Where bits tagged as 'x' is unused and can be undefined.
 * */
struct gds_Function {
    uint8_t descriptor;
    union {
        /* Node is a local variable.
         * Note: resolved only when function is fully constructed. */
        struct {
            uint8_t orderNum; /* is set to 0xff when not resolved */
            const char * name; /* should be never used, when orderNum is resolved. */  
        } asLocalVariable;
        /* Node is numeric value */
        struct gds_Literal * asValue;
        /* Node represents mathematical (binary or unary) operation */
        struct {
            struct gds_Function * l, * r;
        } asMathOperation;
        /* Node represents named function with symbol id and arguments */
        struct {
            struct gds_Function * f;
            struct gds_ArgList * arglist;
        } asFunction;
    } content;
};

/*
 * Mathematical expressions
 */

struct gds_Function * gds_math_new_func_from_const(
        struct gds_Parser *,
        struct gds_Literal * );
struct gds_Function * gds_math_new_func_from_locvar(
        struct gds_Parser *,
        const char * );

struct gds_Function * gds_math(
        struct gds_Parser *,
        char opType,
        struct gds_Function *,
        struct gds_Function *);

struct gds_Function * gds_math_negotiate(
        struct gds_Parser *,
        struct gds_Function * );

void gds_math_function_init(
        struct gds_Parser *,
        struct gds_Function *, /* function node itself */
        struct gds_Function *, /* function root node */
        const char *,
        struct gds_ArgList *);

/** Resolves all local variables inside a function and provides basic validation. */
void gds_math_function_resolve(
        struct gds_Parser *,
        struct gds_Function *);

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_FUNCTION_H */

