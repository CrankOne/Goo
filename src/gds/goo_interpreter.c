# include <stdlib.h>
# include <string.h>

# include "gds/goo_interpreter.h"

# include <stdio.h>  /*XXX*/

/*
 * Constant values
 */

struct GDS_Value *
interpret_integral(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant.\n", s);
    return 0;
}

struct GDS_Value *
interpret_float(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant.\n", s);
    return 0;
}

struct GDS_Value *
memorize_string_literal(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as string literal of length %zd.\n",
            s, strlen(s) );
    return 0;
}

/*
 * Functions
 */

struct GDS_mexpr *
mexpr_from_constant(
        struct GDS_Parser * P,
        struct GDS_Value * val ) {
    printf("XXX: making an anonymous constant expression from numeric value.\n");
    return 0;
}

struct GDS_mexpr *
mexpr_from_logic(
        struct GDS_Parser * P,
        uint8_t v ) {
    printf("XXX: making an anonymous constant expression from logic value.\n");
    return 0;
}

/*
 * Manifestations
 */

struct GDS_expr *
empty_manifest(
        struct GDS_Parser * P,
        void * prhs) {
    printf("XXX: memorize empty manifest with no side effects.\n");
    return 0;
}

struct GDS_expr *
declare_named_constant(
        struct GDS_Parser * P,
        const char * idName,
        struct GDS_Value * val ) {
    printf("XXX: memorize manifest named \"%s\" with expression.\n",
            idName);
    return 0;
}

struct GDS_expr *
eval_math_expression(
        struct GDS_Parser * P,
        struct GDS_mexpr * mexpr ) {
    printf("XXX: making a manifestation from mathematical expression.\n");
    return 0;
}

/*
 * Parser object C-wrapper.
 */

struct GDS_Parser *
gds_parser_new() {
    struct GDS_Parser * pObj = malloc( sizeof(struct GDS_Parser) );
    pObj->scanner = NULL;
    return pObj;
}

void
gds_parser_destroy( struct GDS_Parser * P ) {
    free( P );
}

