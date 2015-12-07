# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <assert.h>
# include "gds/interpreter.h"

# ifdef ENABLE_GDS

# define GDS_PARAMETER__TOKEN_POOL_SIZE 1024

void
gds_context_init_addendum( struct gds_ContextAddendum * C, struct gds_Parser * P ) {
    if( C->tokensPoolPtr ) {
        C->tokensPoolPtr = malloc( GDS_PARAMETER__TOKEN_POOL_SIZE );
        C->tokensPoolEndPtr = C->tokensPoolPtr + GDS_PARAMETER__TOKEN_POOL_SIZE;
    }
    C->tokensPoolEndPtr = C->tokensPoolPtr;
}

char *
gds_lexer_replicate_token( struct gds_Parser * P,
                           const char * srcTokenString ) {
    struct gds_ContextAddendum * C = P->topContextPtr;
    size_t tokenLength = strlen( srcTokenString ) + 1;
    char * c = C->currentTokenEndPtr;
    if( C->currentTokenEndPtr + tokenLength > C->tokensPoolEndPtr ) {
        gds_diagnostic_message( P, gds_kDiagnostic_error,
            "Context addendum exceeded tokens replica pool.");
        /* TODO: switch to heap management instead of abort */
        gds_abort( P, -1 );
    }
    memcpy( c, srcTokenString, tokenLength );
    C->currentTokenEndPtr += tokenLength;
    return c;
}


struct gds_Parser *
gds_parser_new() {
    struct gds_Parser * P = malloc( sizeof(struct gds_Parser) );
    P->scanner = NULL;

    /* Initialize contexts pool */
    bzero( P->rootContexts, sizeof(P->rootContexts) );

    /* Set current to top. */
    P->topContextPtr = P->rootContexts;

    gds_context_init_addendum( P->topContextPtr, P );

    return P;
}

void
gds_parser_context_extension_bgn( struct gds_Parser * P ) {
    /* TODO */
}

void gds_parser_context_extension_end( struct gds_Parser * P ) {
    /* TODO */
}

void
gds_parser_scope_open(  struct gds_Parser * P ) {
    /* TODO */
}

void
gds_parser_scope_close( struct gds_Parser * P ) {
    /* TODO */
}

# endif  /* ENABLE_GDS */

