# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <assert.h>
# include "gds/goo_interpreter.h"

# ifdef ENABLE_GDS

/*
 * Parser object C-wrapper.
 */

# define implement_pool_acq_routines(typeName, size)                        \
struct gds_ ## typeName * gds_parser_new_  ## typeName(                     \
                                                struct gds_Parser * P ) {   \
    if(P->pool_ ## typeName .current >= size) {                             \
        gds_error( P, "Pool depleted." ); }                                 \
    struct gds_ ## typeName * st =                                          \
           P->pool_ ## typeName .instns + P->pool_ ## typeName .current;    \
    ++ P->pool_ ## typeName .current;                                       \
    return st;                                                              \
}
for_all_parser_owned_pools(implement_pool_acq_routines)
# undef implement_pool_acq_routines



# if 0
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
# endif


# define gds_list_implement_routines( StructName, StructPtrType, l )          \
union gds_ ## StructName ## List *                                            \
gds_parser_new_ ## StructName ## List( struct gds_Parser * P ) {              \
    if(P->pool_ ## StructName ## List .current >= l) {                        \
        gds_error( P, "Pool depleted." ); }                                   \
    union gds_ ## StructName ## List * st =                                   \
           P->pool_ ## StructName ## List .instns +                           \
                P->pool_ ## StructName ## List .current;                      \
    ++ P->pool_ ## StructName ## List.current;                                \
    bzero(st, sizeof(union gds_ ## StructName ## List));                      \
    return st;                                                                \
}                                                                             \
union gds_ ## StructName ## List * gds_ ## StructName ## List_append(         \
                                    struct gds_Parser * P,                    \
                                    union gds_ ## StructName ## List * head,  \
                                    StructPtrType entry ) {                   \
    union gds_ ## StructName ## List * ni =                                   \
                             gds_parser_new_ ## StructName ## List (P);       \
    if(head->head.last) {                                                     \
        head->head.last->entry.next = ni;                                     \
    } else {                                                                  \
        head->head.last = head->head.first = ni;                              \
    }                                                                         \
    ni->entry.next= NULL;                                                     \
    ni->entry.this_ = entry;                                                  \
    head->head.last = ni;                                                     \
    return ni; }
for_all_parser_lists(gds_list_implement_routines)
# undef gds_list_implement_routines


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
    # define zero_pool(typeName, size)                      \
    bzero( pObj->pool_ ## typeName . instns ,               \
           sizeof( struct gds_ ## typeName )*size );        \
    pObj->pool_ ## typeName . current = 0;
    for_all_parser_owned_pools(zero_pool)
    # undef zero_pool
    # define zero_pool(StructName, StructPtrType, l)        \
    bzero( pObj->pool_ ## StructName ## List . instns ,     \
           sizeof( union gds_ ## StructName ## List )*l ); \
    pObj->pool_ ## StructName ## List . current = 0;
    for_all_parser_lists(zero_pool)
    # undef zero_pool


    /* Init current scope */
    assert( pObj->thisModule.types = gds_hashtable_new()        );
    assert( pObj->thisModule.functions = gds_hashtable_new()    );
    assert( pObj->thisModule.variables = gds_hashtable_new()    );
    assert( pObj->thisModule.submodules = gds_hashtable_new()   );
    pObj->currentLocArgListChain = NULL;

    /* Init heaps */
    pObj->literals = gds_heap_new();

    return pObj;
}

void
gds_parser_destroy( struct gds_Parser * pObj ) {
    /* Free string literals reentrant buffer */
    free( pObj->strLitBuffer );

    /* Delete current scope */
    if( pObj->thisModule.types      ) { gds_hashtable_free( pObj->thisModule.types     ); }
    if( pObj->thisModule.functions  ) { gds_hashtable_free( pObj->thisModule.functions ); }
    if( pObj->thisModule.variables  ) { gds_hashtable_free( pObj->thisModule.variables ); }
    if( pObj->thisModule.submodules ) { gds_hashtable_free( pObj->thisModule.submodules); }

    /* Free heaps */
    gds_heap_free( pObj->literals );

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
    /*printf(">%s< %p\n", token, P); XXX */

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
    return P->strLitBuffer;
}


/*
 * Symbol table routines.
 */

int8_t
gds_parser_module_resolve_symbol( struct gds_Module * ctx,
                                         const char * key,
                                         void ** result ) {
    /*printf( "!!!XXX: resolving \"%s\".\n", key );*/
    {/* 1) try to resolve symbol as type here. */
        *result = gds_hashtable_search( ctx->types, key );
        if( *result ) {
            return 0x8;
        }
    }
    {/* 2) try to resolve symbol as function here. */
        *result = gds_hashtable_search( ctx->functions, key );
        if( *result ) {
            printf( "SYMTABLE: resolved as a function \"%s\".\n", key );
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
gds_parser_push_locvar_arglist(
        struct gds_Parser * P,
         union gds_ArgList * al ) {
    if( NULL == P->currentLocArgListChain ) {
        P->currentLocArgListChain = P->argListChains;
    } else if( P->currentLocArgListChain >=
        P->argListChains + sizeof(P->argListChains)/sizeof(union gds_ArgList *) ) {
        gds_error( P, "Local argument lists pool depleted for nested declarations." );
    } else {
        (P->currentLocArgListChain)++;
    }
    *((P->currentLocArgListChain)) = al;
}

void
gds_parser_pop_locvar_arglist(
        struct gds_Parser * P ) {
    if( NULL == P->currentLocArgListChain ) {
        /* Logical error we must be aware of. */
        gds_error( P, "Local variable scope exit invoked when not in scope." );
    } else if( P->currentLocArgListChain == P->argListChains ) {
        P->currentLocArgListChain = NULL;
    } else {
        (P->currentLocArgListChain)--;
    }
}

struct gds_Function *
gds_parser_deepcopy_function(
        struct gds_Parser * P,
        struct gds_Function * f ) {
    struct gds_Function * copiedF = 
        gds_function_heapcopy( P, f );
    gds_hashtable_replace( P->thisModule.functions,
                          f->content.asFunction.name,
                          copiedF );
    free( f->content.asFunction.name );
    return copiedF;
}

# endif  /* ENABLE_GDS */

