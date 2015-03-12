# include <stdlib.h>
# include <string.h>

# include "gds/goo_interpreter.h"

# include <stdio.h>  /*XXX*/

void gds_error( struct gds_Parser * P, const char * det );
void gds_warn( struct gds_Parser * P, const char * det );

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

struct gds_Value *
interpret_bin_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in binary form.\n", s);
    return 0;
}

struct gds_Value *
interpret_oct_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in octal form.\n", s);
    return 0;
}

struct gds_Value *
interpret_hex_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in hexidecimal form.\n", s);
    return 0;
}

struct gds_Value *
interpret_esc_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in escape sequence form.\n", s);
    return 0;
}

struct gds_Value *
interpret_dec_integral(
        struct gds_Parser * P,
        const char * s ){
    /* TODO:
     *  1) Check truncation errors here (strtoi has special behaviour);
     *  2) Check extra symbols on tail and warn, if they're invalid.
     */
    struct gds_Value * v = gds_new_empty_value(P);
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

struct gds_Value *
interpret_float_dec(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in decimal form.\n", s);
    return 0;
}

struct gds_Value *
interpret_float_hex(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in hexidecimal form.\n", s);
    return 0;
}

/* string literal */

struct gds_Value *
memorize_string_literal(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as string literal of length %zd.\n",
            s, strlen(s) );
    return 0;
}

/*
 * Math function
 */

# define for_all_gds_math_node_descriptor_postfix( m )  \
    m( gds_math_nodeNumeric,    0x2 )                   \
    m( gds_math_nodeLocVar,     0x1 )                   \
    m( gds_math_nodeMathOp,     0x3 )

# define for_all_gds_math_node_masks( m )           \
    m( gds_math_nodeTypeMask,   0x3  )              \
    m( gds_math_nodeMathOpMask, 0x3c )

# define for_all_gds_binary_math_operators( m )     \
    m( gds_math_summation_c        , 0x03, '+' )    \
    m( gds_math_subtraction_c      , 0x07, '-' )    \
    m( gds_math_mutliplication_c   , 0x0b, '*' )    \
    m( gds_math_division_c         , 0x0f, '/' )    \
    m( gds_math_power_c            , 0x13, '^' )    \
    m( gds_math_modulo_c           , 0x17, '%' )    \
    m( gds_math_dot_c              , 0x23, '.' )

static const uint8_t
# define declare_const_b( name, code ) name = code,
    for_all_gds_math_node_descriptor_postfix(declare_const_b)
    for_all_gds_math_node_masks(declare_const_b)
# undef declare_const_b
# define declare_const_b( name, code, symbol ) name = code,
    for_all_gds_binary_math_operators(declare_const_b)
# undef declare_const_b
    gds_math_unegotiation_c     = 0x1f;

struct gds_Function *
gds_math_new_func_from_const(
        struct gds_Parser * P,
        struct gds_Value * constVal ) {
    struct gds_Function * node = gds_parser_math_function_new(P);

    node->descriptor = gds_math_nodeNumeric;
    node->content.asValue = constVal;

    return node;
}

struct gds_Function *
gds_math_new_func_from_locvar(
        struct gds_Parser * P,
        struct gds_Value * locVar ) {
    struct gds_Function * node = gds_parser_math_function_new(P);

    node->descriptor = gds_math_nodeLocVar;
    node->content.asLocalVariable.orderNum = 1 /* TODO: acquire locvar name */ ;
    fprintf( stderr, "Locvar acquizition is unimplemented yet. Setting to $1.\n" );

    return node;
}

struct gds_Function *
gds_math(
        struct gds_Parser * P,
        char opType,
        struct gds_Function * l,
        struct gds_Function * r) {
    struct gds_Function * node = gds_parser_math_function_new(P);
    node->content.asMathOperation.l = l;
    node->content.asMathOperation.r = r;
    switch( opType ) {
        # define case_( name, code, glyph ) \
            case glyph : {                  \
                node->descriptor = name;    \
            } break;
        for_all_gds_binary_math_operators(case_);
        # undef case_
        default: {
            fprintf( stderr,
                    "Unknown symbol '%c' interpreted as math operator: internal parser error.\n",
                    opType );
        }
    }; /* switch( opType ) */
    return node;
}

struct gds_Function *
gds_math_negotiate(
        struct gds_Parser * P,
        struct gds_Function * o) {
    struct gds_Function * node = gds_parser_math_function_new(P);
    node->descriptor = gds_math_unegotiation_c;
    node->content.asMathOperation.l = o;
    node->content.asMathOperation.r = NULL;
    return node;
}

/*
 * Manifestations
 */

struct GDS_expr *
empty_manifest(
        struct gds_Parser * P,
        void * prhs) {
    /* TODO: there should be a warning about absence of side effects. */
    printf("XXX: memorize empty manifest with no side effects.\n");
    return 0;
}

struct GDS_expr *
declare_named_constant(
        struct gds_Parser * P,
        const char * idName,
        struct gds_Value * val ) {
    printf("XXX: memorize manifest named \"%s\" with expression.\n",
            idName);
    return 0;
}

/*
 * Parser object C-wrapper.
 */

struct gds_Parser *
gds_parser_new() {
    struct gds_Parser * pObj = malloc( sizeof(struct gds_Parser) );
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
gds_parser_destroy( struct gds_Parser * pObj ) {
    free( pObj->strLitBuffer );
    free( pObj );
}

void
gds_parser_set_filename( struct gds_Parser * P,
                         const char * filename ) {
    strncpy(P->currentFilename, filename, 128);
    P->currentFilename[127] = '\0';
}


char *
gds_parser_replicate_token(
        struct gds_Parser * P,
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

struct gds_Value *
gds_new_empty_value( struct gds_Parser * P ) {
    /* TODO use kind of pool here. */
     struct gds_Value * vPtr = malloc( sizeof(struct gds_Value) );
     bzero( vPtr, sizeof(struct gds_Value) );
     return vPtr;
}

void
gds_parser_free_buffer( struct gds_Parser * P ) {
    P->cScope->lastReplica = P->cScope->tokenReplicasBf;
    *(P->cScope->lastReplica) = '\0';
}

void
gds_parser_str_lit( struct gds_Parser * P,
                    const char * strb ) {
    /* TODO: get first char of strb to determine which type
     * of string is to be used (U,u,L,l, etc).
     */
    P->strLitBufferC = P->strLitBuffer;
}

void
gds_parser_append_lstr_lit( struct gds_Parser * P,
                            const char * c ) {
    *(P->strLitBufferC ++) = *c;
}

char *
gds_parser_opt_lstr_lit( struct gds_Parser * P ) {
    *(P->strLitBufferC ++) = '\0';
    /*printf("Lexing STRING_LITERAL done: \"%s\".\n", P->strLitBuffer);*/
    return P->strLitBuffer;
}

