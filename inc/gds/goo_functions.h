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

/** Returns new void arglist entry. */
union gds_ArgList * gds_math_new_arglist_entry(
        struct gds_Parser * );
/** Sets name of provided arglist entry. */
union gds_ArgList * gds_math_set_arglist_name(
        struct gds_Parser *,
        union gds_ArgList *,
        const char *);
/** Appends arglist with new arglist entry */
union gds_ArgList * gds_math_append_arglist(
        struct gds_Parser *,
        union gds_ArgList *,
        union gds_ArgList *);

/**@struct gds_Function
 * @brief Mathematical function description.
 *
 * All parsed mathematical expressions are presented in
 * intermediate form as a tree-like topological structure.
 * 
 * Each node has a descriptor encoding how this node is
 * to be interpreted further, on evaluation:
 *      node is a local variable            000x xxxx   0x00
 *      node is a numeric value             001x xxxx   0x20
 *      node represents a math operation    010x yyyy   0x40
 *        * Unary:
 *          - unary negotiation             010x 0000   0x40
 *          - abs                           010x 1111   0x4f
 *          - factorial                     010x 1010   0x4a
 *        * Binary:
 *          - summation                     010x 0011   0x43
 *          - subtraction                   010x 0010   0x42
 *          - multiplication                010x 0100   0x44
 *          - division                      010x 0101   0x45
 *          - power (exponentiation)        010x 0111   0x47
 *          - modulo (division remainder)   010x 0110   0x46
 *          - dot (dot product)             010x 1000   0x48
 *      node represents a logic             011y yyyy   0x60
 *        * Unary
 *          - inversion                     0110 0000   0x60
 *        * Binary
 *          - less                          0110 0001   0x61
 *          - less or equal                 0110 0010   0x62
 *          - greater                       0110 0101   0x65
 *          - grater or equal               0110 0100   0x64
 *          - exact equation                0110 1110   0x6e
 *          - nearly equals                 0110 1010   0x6a
 *          - logical AND                   0110 1000   0x68
 *          - logical OR                    0110 1001   0x69
 *          - logical XOR                   0110 1011   0x6b
 *        * Ternary
 *          - < <                           0111 1110   0x7e
 *          - < <=                          0111 1100   0x7c
 *          - <= <                          0111 1010   0x7a
 *          - <= <=                         0111 1000   0x78
 *          - > >                           0111 0110   0x76
 *          - > >=                          0111 0100   0x74
 *          - >= >                          0111 0010   0x72
 *          - >= >=                         0111 0000   0x70
 *          - nearly equals w eps           0111 0001   0x71
 *      node represents a function          111x xxxx
 *          * GDS-math function             1110 0001   0xf1
 *          * Piecewise math                1110 0010   0xf2
 *          * third-party                   1110 0011   0xf3
 *      node is a composite object          100x xxxx   0x80
 *  Where bits tagged as 'x' is unused and can be undefined and y is
 *  a meaningful bytes in subcategory.
 *  Masks:
 *      0xE0 -- last 3 bits
 *          defines, whether node is a locvar (0x0), numeric value (0x20),
 *          math operation (0x40), logic operation (0x60) or an entire
 *          self-consistent function (0xE0).
 *      0xEF -- supresses insignificant 5-th bit for determining math
 *              operations.
 * */
struct gds_Function {
    uint8_t descriptor;
    union {
        /* Node is a local variable.
         * Note: resolved only when function is fully constructed. */
        struct {
            uint8_t orderNum; /* is set to 0xff when not resolved */
        } asLocalVariable;
        /* Node is numeric value */
        struct gds_Literal * asValue;
        /* Node represents mathematical (binary or unary) operation */
        struct {
            struct gds_Function * l, * r;
        } asMathOperation;
        /* Node represents logical (binary or unary) operation */
        struct {
            struct gds_Function * l, * r;
        } asLogicalOperation;
        /* Node represents named function with symbol id and arguments */
        struct {
            char * name;  /*Note: heap-allocated.*/
            struct gds_Function * f;
            union gds_ArgList * arglist;
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
        uint8_t lvno );

struct gds_Function * gds_math(
        struct gds_Parser *,
        char opType,
        struct gds_Function *,
        struct gds_Function *);

struct gds_Function *
gds_math_logical_binary_op(
        struct gds_Parser *,
        char,
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
        union gds_ArgList *);

struct gds_Function *
gds_function_heapcopy( struct gds_Parser *, const struct gds_Function * );
void
gds_function_heapfree( struct gds_Parser *, struct gds_Function * );

struct gds_Expr *
gds_expr_from_func_decl(
        struct gds_Parser * P,
        struct gds_Function * f );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_FUNCTION_H */

