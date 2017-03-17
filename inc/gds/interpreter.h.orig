/**\file goo_interpreter.h
 * \brief File containing general parser structure and related routines.
 *
 * Parser object stores all data associated with current GDS context.
 * Its routines manage intermediate data lifetime which are related
 * to corresponding generated lexer/parser.
 */

# ifndef H_GOO_INTERPRETER_H
# define H_GOO_INTERPRETER_H

# include "goo_config.h"

# ifdef ENABLE_GDS

# include <stdio.h>
# include "goo_ansi_escseq.h"
# include "gds/arithmetic_eval.h"
# include "gds/diagnostic.h"

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct gds_Parser;

/**\struct gds_ContextAddendum
 * \brief A context complementary fragment (semantic scope, actually).
 *
 * During expression processing the parser itself keeps track of symbols
 * as a sets of named objects whose lifetime is bound to certain scopes.
 * This scopes, internally, are presented with this gds_ContextAddendum
 * structure.
 *
 * The token replication mechanism does not imply freeng token inside an
 * active context addendum, so once allocated they're available until
 * addendum will be freed.
 *
 * Context addendums are managed via following functions:
 *    * \ref gds_context_init_addendum()
 *    * \ref gds_parser_scope_open()
 *    * \ref gds_parser_scope_close()
 *    * \ref gds_parser_context_extension_bgn()
 *    * \ref gds_parser_context_extension_end()
 */
struct gds_ContextAddendum {
    char * tokensPoolPtr,
         * currentTokenEndPtr,
         * tokensPoolEndPtr;
    /* ... */
    struct gds_ContextAddendum * next;
};

void gds_context_init_addendum( struct gds_ContextAddendum *, struct gds_Parser * );

/* \func gds_lexer_replicate_token
 * \brief Uses current context to allocate token replica.
 *
 * Is mostly used by lexer inside a \ref MEMTOKEN macro.
 *
 * \see gds_ContextAddendum
 */
char * gds_lexer_replicate_token( struct gds_Parser *, const char * );

/**\struct gds_Parser
 * 
 * The parser struct instance is unique for each parsing/evaluation
 * thread. It keeps track of data lifetimes, symbol resolution and
 * so on. It provides the mechanism of stacking contexts while actual
 * semantic analyser switching it context.
 *
 * Parser also keeps track the current lexical scanner instance and
 * current lexer's location. Pointer to current parser is stored
 * in FLEX/LEX `extra` section.
 */
struct gds_Parser {
    FILE * runtimeLogStream,
         * stdoutStream,
         * errStream
         ;

    /* YACC/FLEX section {{{ */
    void * scanner;
    char currentFilename[128];
    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* }}} */

    struct gds_ContextAddendum rootContexts[256],
                               * topContextPtr;
};

void gds_parser_context_extension_bgn( struct gds_Parser * );
void gds_parser_context_extension_end( struct gds_Parser * );
void gds_parser_scope_open(  struct gds_Parser * );
void gds_parser_scope_close( struct gds_Parser * );

/*
 * Constructors
 */

struct ArithmeticConstant * gds_context_new_ArithmeticConstant( struct gds_Parser * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */
# endif  /* H_GOO_INTERPRETER_H */
