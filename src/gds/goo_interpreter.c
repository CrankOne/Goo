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

    /* Zero all pools */
    # define zero_pool(typeName, size) \
    bzero( pObj->pool_ ## typeName . instns , sizeof( struct gds_ ## typeName )*size ); \
    pObj->pool_ ## typeName . current = 0;
    for_all_parser_stacked_pools(zero_pool)
    for_all_parser_owned_pools(zero_pool)
    # undef zero_pool

    /* Init current scope */
    pObj->thisModule.types = gds_hashtable_new();
    pObj->thisModule.functions = gds_hashtable_new();
    pObj->thisModule.variables = gds_hashtable_new();
    pObj->thisModule.submodules = gds_hashtable_new();

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


/*
 * Symbol table routines.
 */

int8_t
gds_parser_module_resolve_symbol( struct gds_Module * ctx,
                                         const char * key,
                                         void ** result ) {
    {/* 1) try to resolve symbol as type here. */
        *result = gds_hashtable_search( ctx->types, key );
        if( *result ) {
            return 0x8;
        }
    }
    {/* 2) try to resolve symbol as function here. */
        *result = gds_hashtable_search( ctx->functions, key );
        if( *result ) {
            return 0x4;
        }
    }
    {/* 3) try to resolve symbol as variable here. */
        *result = gds_hashtable_search( ctx->variables, key );
        if( *result ) {
            return 0x2;
        }
    }
    {/* 4) try to resolve symbol as module here. */
        *result = gds_hashtable_search( ctx->submodules, key );
        if( *result ) {
            return 0x1;
        }
    }
    return 0x0;
}

void
gds_parser_pop_locvar_arglist(
        struct gds_Parser * P ) {
    /* TODO */
}

void
gds_parser_push_locvar_arglist(
        struct gds_Parser * P,
        struct gds_ArgList * al ) {
    /* TODO */
}

struct gds_Expr *
gds_parser_math_function_declare(
        struct gds_Parser * P,
        struct gds_Function * f ) {
    /* TODO */
    return NULL;
}

struct gds_Expr *
gds_parser_variables_declare(
        struct gds_Parser * P,
        struct gds_VarList * vl ) {
    /* TODO */
    return NULL;
}

# endif  /* ENABLE_GDS */

