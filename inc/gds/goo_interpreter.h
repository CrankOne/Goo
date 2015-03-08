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

struct GDS_Value * interpret_integral(  const char * );
struct GDS_Value * interpret_float(const char * );

/*
 * Functions
 */

struct GDS_mexpr {
    /* ... */
};

struct GDS_mexpr * mexpr_from_constant( struct GDS_Value * );
struct GDS_mexpr * mexpr_from_logic( uint8_t );

/*
 * Manifestations
 */

struct GDS_expr {
    /* ... */
};

struct GDS_expr * empty_manifest(void*);
struct GDS_expr * declare_named_constant(const char *, struct GDS_expr * );

struct GDS_expr * eval_math_expression( struct GDS_mexpr * );

/*
 * Parser object C-wrapper.
 */

struct GDS_Parser {
    void * scanner;
    /* ... */
};

struct GDS_Parser * gds_parser_new();
void gds_parser_destroy( struct GDS_Parser * );

void gds_eval_file(   struct GDS_Parser *, FILE * );
void gds_eval_string( struct GDS_Parser *, const char * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* H_GOO_INTERPRETER_H */

