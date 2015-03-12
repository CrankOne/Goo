# ifndef H_GOO_INTERPRETER_H
# define H_GOO_INTERPRETER_H

# include <stdio.h>  /* for FILE structure */
# include "goo_config.h"
# include "goo_types.h"

/**@file goo_interpreter.h
 * @brief File contains general parser state machine routines.
 *
 * This file is supposed to be used both in C and C++ routines.
 */

# ifdef __cplusplus
namespace goo {
namespace ds {

class Parser {
    // ...
};

}  // namespace ds
}  // namespace goo
# endif

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct gds_Parser;

/*
 * Constant values.
 */

struct gds_Value {
    enum {
        # define declare_enum( num, cname, gname, gdsname ) \
        gdsname ## _CT = num,
        for_all_atomic_datatypes(declare_enum)
        # undef declare_enum
        logic_CT = 0xc,
        string_CT = 0x83,
        struct_CT = 0x84,
    } typecode;
    union {
        # define declare_datfield( num, cname, gname, gdsname ) \
        cname gdsname ## Val;
        for_all_atomic_datatypes(declare_datfield)
        # undef declare_datfield
        uint8_t logicValPtr;
        char * stringValPtr;
        void * structValPtr;
    } data;
};

struct gds_Value * interpret_bin_integral(
    struct gds_Parser *,
    const char * );
struct gds_Value * interpret_oct_integral(
    struct gds_Parser *,
    const char * );
struct gds_Value * interpret_hex_integral(
    struct gds_Parser *,
    const char * );
struct gds_Value * interpret_esc_integral(
    struct gds_Parser *,
    const char * );
struct gds_Value * interpret_dec_integral(
    struct gds_Parser *,
    const char * );


struct gds_Value * interpret_float_dec(
    struct gds_Parser *,
    const char * );
struct gds_Value * interpret_float_hex(
    struct gds_Parser *,
    const char * );

struct gds_Value *
memorize_string_literal(
        struct gds_Parser * P,
        const char * s );

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
        struct gds_Value * asValue;
        /* Node represents mathematical (binary or unary) operation */
        struct {
            struct gds_Function * l, * r;
        } asMathOperation;
    } content;
};

struct gds_Function * gds_math_new_func_from_const(
        struct gds_Parser *,
        struct gds_Value * );
struct gds_Function * gds_math_new_func_from_locvar(
        struct gds_Parser *,
        struct gds_Value * );

struct gds_Function * gds_math(
        struct gds_Parser *,
        char opType,
        struct gds_Function *,
        struct gds_Function *);

struct gds_Function * gds_math_negotiate(
        struct gds_Parser *,
        struct gds_Function * );

/*
 * Manifestations
 */

struct GDS_expr {
    /* ... */
};

struct GDS_expr * empty_manifest(
    struct gds_Parser *,
    void*);
struct GDS_expr * declare_named_constant(
    struct gds_Parser *,
    const char *, struct gds_Value * );

/*
 * Parser object C-wrapper.
 */

struct gds_Parser {
    void * scanner;
    struct ReplicaBuffer {
        char tokenReplicasBf[GDS_PARSER_EXPR_REPLICA_BUF_LENGTH];
        char * lastReplica;
        const char * tokenReplicasBfEnd;
    } buffers[GDS_PARSER_NSCOPES],
      * cScope;
    char * strLitBuffer,
         * strLitBufferC;

    char currentFilename[128];

    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* ... */
};

void gds_parser_set_filename( struct gds_Parser *,
                              const char * );

# ifdef __cplusplus
extern "C" {
# endif

/** Raises parser error. */
void gds_parser_raise_error(
        struct gds_Parser * P,
        const char * filename,
        uint16_t line,
        uint16_t columnBgn,
        uint16_t columnEnd,
        const char * details );

void gds_parser_warning(
        struct gds_Parser * P,
        const char * filename,
        uint16_t line,
        uint16_t columnBgn,
        uint16_t columnEnd,
        const char * details );

# ifdef __cplusplus
}  // extern "C"
# endif

struct gds_Parser * gds_parser_new();
void gds_parser_destroy( struct gds_Parser * );
char * gds_parser_replicate_token( struct gds_Parser *, const char * );
struct gds_Value * gds_new_empty_value( struct gds_Parser * );
void gds_parser_free_buffer( struct gds_Parser * );

/* Re-inits string literal buffer. */
void gds_parser_str_lit( struct gds_Parser *, const char * );
/* Appends string in string literal buffer with given character. */
void gds_parser_append_lstr_lit( struct gds_Parser *, const char * );
/* Returns string literal bufer begin. */
char * gds_parser_opt_lstr_lit( struct gds_Parser * );

/* Math functions */
struct gds_Function * gds_parser_math_function_new( struct gds_Parser * );  /*TODO*/

/*
 * Flex-related routines.
 * See scanner definition file for implementation.
 */

void gds_eval_file(   struct gds_Parser *, FILE * );
void gds_eval_string( struct gds_Parser *, const char * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* H_GOO_INTERPRETER_H */

