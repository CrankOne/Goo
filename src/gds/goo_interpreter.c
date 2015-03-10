# include <stdlib.h>
# include <string.h>

# include "gds/goo_interpreter.h"

# include <stdio.h>  /*XXX*/

void gds_error( struct GDS_Parser * P, const char * det );
void gds_warn( struct GDS_Parser * P, const char * det );

/*
 * Constant values
 */

/* integral */

/* Flags:
 *  u    - 0x1
 *  l1   - 0x2
 *  l2   - 0x4
 * so:
 *  ull  - 7
 *  ul   - 3
 *  u    - 1
 *  ll   - 6
 *  l    - 2
 */
static uint8_t
integral_parse_typemod_postfix( const char * intStr ) {
    uint8_t flags = 0;
    for( const char * c = intStr;
                    *c != '\0'; ++c ) {
        if( 'u' == *c || 'U' == *c ) {
            flags |= 0x1;
        }
        if( 'l' == *c || 'L' == *c ) {
            if( flags & 0x2 ) {
                flags |= 0x4;
            } else {
                flags |= 0x2;
            }
        }
    }
    return flags;
}

struct GDS_Value *
interpret_bin_integral(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in binary form.\n", s);
    return 0;
}

struct GDS_Value *
interpret_oct_integral(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in octal form.\n", s);
    return 0;
}

struct GDS_Value *
interpret_hex_integral(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in hexidecimal form.\n", s);
    return 0;
}

struct GDS_Value *
interpret_esc_integral(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in escape sequence form.\n", s);
    return 0;
}

struct GDS_Value *
interpret_dec_integral(
        struct GDS_Parser * P,
        const char * s ){
    /* TODO:
     *  1) Check truncation errors here (strtoi has special behaviour);
     *  2) Check extra symbols on tail and warn, if they're invalid.
     */
    struct GDS_Value * v = gds_new_empty_value(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endCPtr;
    if( typemod & 0x2 ) {
        if( typemod & 0x4 ) {
            # ifdef EXTENDED_TYPES
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoull( s, &endCPtr, 10 );
                v->typecode = UInt128_CT;
                printf("XXX: \"%s\" dec int, uul.\n", s);
            } else {
                v->data.Int64Val = strtoll( s, &endCPtr, 10 );
                v->typecode = UInt128_CT;
                printf("XXX: \"%s\" dec int, ll.\n", s);
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error(P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt32Val = strtoul( s, &endCPtr, 10 );
                v->typecode = UInt64_CT;
                printf("XXX: \"%s\" dec int, ul.\n", s);
            } else {
                v->data.Int32Val = strtol( s, &endCPtr, 10 );
                v->typecode = UInt64_CT;
                printf("XXX: \"%s\" dec int, l.\n", s);
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = strtoul( s, &endCPtr, 10 );
            v->typecode = UInt32_CT;
            printf("XXX: \"%s\" dec int, u.\n", s);
        } else {
            v->data.Int32Val = strtol( s, &endCPtr, 10 );
            v->typecode = UInt32_CT;
            printf("XXX: \"%s\" dec int.\n", s);
        }
    }
    return v;
}

/* float */

struct GDS_Value *
interpret_float_dec(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in decimal form.\n", s);
    return 0;
}

struct GDS_Value *
interpret_float_hex(
        struct GDS_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in hexidecimal form.\n", s);
    return 0;
}

/* string literal */

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
    /* TODO: there should be a warning about absence of side effects. */
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

void
gds_parser_set_filename( struct GDS_Parser * P,
                         const char * filename ) {
    strncpy(P->currentFilename, filename, 128);
    P->currentFilename[127] = '\0';
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

struct GDS_Value *
gds_new_empty_value( struct GDS_Parser * P ) {
    /* TODO use kind of pool here. */
     struct GDS_Value * vPtr = malloc( sizeof(struct GDS_Value) );
     bzero( vPtr, sizeof(struct GDS_Value) );
     return vPtr;
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

