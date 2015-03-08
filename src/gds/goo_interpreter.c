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
    printf("Parser allocated: %p\n", pObj);  /* XXX */

    pObj->scanner = NULL;

    /* Initialize tokens replicator */
    for( pObj->cScope  = pObj->buffers;
         pObj->cScope != pObj->buffers + GDS_PARSER_NSCOPES; pObj->cScope ++) {
        bzero( pObj->cScope->tokenReplicasBf, GDS_PARSER_EXPR_REPLICA_BUF_LENGTH );
        pObj->cScope->tokenReplicasBfEnd = 
            pObj->cScope->tokenReplicasBf + GDS_PARSER_EXPR_REPLICA_BUF_LENGTH;
        pObj->cScope->lastReplica = pObj->cScope->tokenReplicasBf;
    }
    /* Set current to top. */
    pObj->cScope  = pObj->buffers;

    /* Allocate string literals buffer */
    pObj->strLitBuffer = pObj->strLitBufferC = malloc(GDS_PARSER_STRING_BUF_LEN);
    bzero(pObj->strLitBuffer, GDS_PARSER_STRING_BUF_LEN);

    return pObj;
}

void
gds_parser_destroy( struct GDS_Parser * pObj ) {
    free( pObj->strLitBuffer );
    free( pObj );
}


char *
gds_parser_replicate_token(
        struct GDS_Parser * P,
        const char * token ) {
    printf(">%s< %p\n", token, P);  /* XXX */

    char * currentReplicaBegin = P->cScope->lastReplica;
    const char * c;

    for( c = token; '\0' != *c; c++, P->cScope->lastReplica ++ ) {
        *(P->cScope->lastReplica) = *c;
    }
    *(P->cScope->lastReplica ++) = '\0';

    return currentReplicaBegin;
}

void
gds_parser_free_buffer( struct GDS_Parser * P ) {
    P->cScope->lastReplica = P->cScope->tokenReplicasBf;
    *(P->cScope->lastReplica) = '\0';
}

void
gds_parser_str_lit( struct GDS_Parser * P,
                    const char * strb ) {
    /* TODO: get first char of strb to determine which type
     * of string is to be used (U,u,L,l, etc).
     */
    P->strLitBufferC = P->strLitBuffer;
}

void
gds_parser_append_lstr_lit( struct GDS_Parser * P,
                            const char * c ) {
    *(P->strLitBufferC ++) = *c;
}

char *
gds_parser_opt_lstr_lit( struct GDS_Parser * P ) {
    *(P->strLitBufferC ++) = '\0';
    /*printf("Lexing STRING_LITERAL done: \"%s\".\n", P->strLitBuffer);*/
    return P->strLitBuffer;
}

