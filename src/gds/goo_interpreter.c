# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include "gds/goo_interpreter.h"

# ifdef ENABLE_GDS

/*
 * Parser object C-wrapper.
 */

# define implement_pool_acq_routines(typeName, size)                        \
struct gds_ ## typeName * gds_parser_new_  ## typeName(                     \
                                                struct gds_Parser * P ) {   \
    if(P->pool_ ## typeName .current >= size) {                             \
        gds_error( P, "Pool overflow" ); }                                  \
    struct gds_ ## typeName * st =                                          \
           P->pool_ ## typeName .instns + P->pool_ ## typeName .current;    \
    ++ P->pool_ ## typeName .current;                                       \
    return st;                                                              \
}
for_all_parser_stacked_pools(implement_pool_acq_routines)
for_all_parser_owned_pools(implement_pool_acq_routines)
# undef implement_pool_acq_routines

# define implement_pool_free_routines(typeName, size)                       \
void gds_parser_free_ ## typeName( struct gds_Parser * P ) {                \
        if( ! (P->pool_ ## typeName .current)) {                            \
        gds_error( P, "Internal parser logic error!" ); }                   \
    --(P->pool_ ## typeName .current);                                      \
    bzero( P->pool_ ## typeName .instns + P->pool_ ## typeName .current,    \
           sizeof(struct gds_ ## typeName) );                               \
}
for_all_parser_stacked_pools(implement_pool_free_routines)
# undef implement_pool_free_routines

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

# endif  /* ENABLE_GDS */

