# include <stdlib.h>

# include "gds/goo_interpreter.h"

# include <stdio.h>  /*XXX*/

/*
 * Constant values
 */

struct GDS_Value *
interpret_dec(  const char * s ){
    printf("XXX: parse \"%s\" as dec.\n", s);
    return 0;
}

struct GDS_Value *
interpret_dec_u(const char * s ){
    printf("XXX: parse \"%s\" as decu.\n", s);
    return 0;
}

struct GDS_Value *
interpret_hex(  const char * s ){
    printf("XXX: parse \"%s\" as hex.\n", s);
    return 0;
}

struct GDS_Value *
interpret_oct(  const char * s ){
    printf("XXX: parse \"%s\" as oct.\n", s);
    return 0;
}

struct GDS_Value *
interpret_bin(  const char * s ){
    printf("XXX: parse \"%s\" as bin.\n", s);
    return 0;
}

struct GDS_Value *
interpret_float(const char * s ){
    printf("XXX: parse \"%s\" as float.\n", s);
    return 0;
}

struct GDS_Value *
interpret_scifl(const char * s ){
    printf("XXX: parse \"%s\" as sci-float.\n", s);
    return 0;
}

/*
 * Functions
 */

struct GDS_mexpr *
mexpr_from_constant( struct GDS_Value * val ) {
    printf("XXX: making an anonymous constant expression from value.\n");
    return 0;
}

/*
 * Manifestations
 */

struct GDS_expr *
eval_math_expression( struct GDS_mexpr * mexpr ) {
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

