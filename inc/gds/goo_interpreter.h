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

struct GDS_Parser;

/*
 * Constant values.
 */

struct GDS_Value {
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

struct GDS_Value * interpret_integral(
    struct GDS_Parser *,
    const char * );
struct GDS_Value * interpret_float(
    struct GDS_Parser *,
    const char * );

/*
 * Functions
 */

struct GDS_mexpr {
    /* ... */
};

struct GDS_mexpr * mexpr_from_constant(
    struct GDS_Parser *,
    struct GDS_Value * );
struct GDS_mexpr * mexpr_from_logic(
    struct GDS_Parser *,
    uint8_t );

/*
 * Manifestations
 */

struct GDS_expr {
    /* ... */
};

struct GDS_expr * empty_manifest(
    struct GDS_Parser *,
    void*);
struct GDS_expr * declare_named_constant(
    struct GDS_Parser *,
    const char *, struct GDS_Value * );
struct GDS_expr * eval_math_expression(
    struct GDS_Parser *,
    struct GDS_mexpr * );

/*
 * Parser object C-wrapper.
 */

struct GDS_Parser {
    void * scanner;
    struct ReplicaBuffer {
        char tokenReplicasBf[GDS_PARSER_EXPR_REPLICA_BUF_LENGTH];
        char * lastReplica;
        const char * tokenReplicasBfEnd;
    } buffers[GDS_PARSER_NSCOPES],
      * cScope;
    char * strLitBuffer,
         * strLitBufferC;
    /* ... */
};

struct GDS_Parser * gds_parser_new();
void gds_parser_destroy( struct GDS_Parser * );

char * gds_parser_replicate_token( struct GDS_Parser *, const char * );
void gds_parser_free_buffer( struct GDS_Parser * );

/* Re-inits string literal buffer. */
void gds_parser_str_lit( struct GDS_Parser *, const char * );
/* Appends string in string literal buffer with given character. */
void gds_parser_append_lstr_lit( struct GDS_Parser *, const char * );
/* Returns string literal bufer begin. */
char * gds_parser_opt_lstr_lit( struct GDS_Parser * );

/*
 * Flex-related routines.
 * See scanner definition file for implementation.
 */

void gds_eval_file(   struct GDS_Parser *, FILE * );
void gds_eval_string( struct GDS_Parser *, const char * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* H_GOO_INTERPRETER_H */

