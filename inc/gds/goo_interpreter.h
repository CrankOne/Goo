# ifndef H_GOO_INTERPRETER_H
# define H_GOO_INTERPRETER_H

# include "goo_config.h"
# include "goo_types.h"

# ifdef ENABLE_GDS

/**@file goo_interpreter.h
 * @brief File containing parser structure and its generic routines.
 *
 * Parser object stores all data associated with current GDS context.
 * Its routines manage intermediate data lifetime which are related
 * to corresponding generated lexer/parser routines.
 *
 * Namely, the parser struct:
 *  - Holds pool of tokens replicas. They're alive from the beginning
 *    of expression until the ';' terminal given.
 *  - Holds pool for long string literals. They're alive during whole
 *    parser lifetime.
 *  - Holds pool of parsed literals structures. They're alive during
 *    whole parser lifetime.
 *  - Hold a symbol tables, which are:
 *  -- Modules symbol tables(*), like `Math::` or `Units::`.
 *  -- Current session symbol table (in session-context -- variables
 *     and functions).
 *  -- Local symbol table that is cleared just after current function
 *     declaration ends.
 *
 * (*) -- each module's symbol table consists of variables and
 *        functions just as the current session symtable.
 *
 * Parser also keeps track the current lexical scanner instance and
 * current lexer's location. Pointer to current parser is stored
 * in FLEX/LEX `extra` section.
 */

# include <stdio.h>

# include "goo_gds_forwards.h"
# include "goo_literals.h"
# include "goo_functions.h"
/* ... */

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct gds_Hashtable;
struct gds_Module;

/*
 * Parser object C-wrapper.
 */

struct gds_Parser {
    /* YACC/FLEX section {{{ */
    void * scanner;
    char currentFilename[128];
    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* }}} */

    struct ReplicaBuffer {
        char tokenReplicasBf[GDS_PARSER_EXPR_REPLICA_BUF_LENGTH];
        char * lastReplica;
        const char * tokenReplicasBfEnd;
    } buffers[GDS_PARSER_NSCOPES],
      * cScope;
    char * strLitBuffer,
         * strLitBufferC;
    /* ... */
};

/** Sets current filename. Should be called before lexical analysis (yylex()). */
void gds_parser_set_filename( struct gds_Parser *,
                              const char * );

/** Creates new instance of GDS parser. */
struct gds_Parser * gds_parser_new();
/** Destroys a parser instance. */
void gds_parser_destroy( struct gds_Parser * );
/** Frees current parser tokens buffer. */
void gds_parser_free_buffer( struct gds_Parser * );

/*
 * Flex-related routines.
 * See scanner definition file for implementation.
 */

void gds_eval_file(   struct gds_Parser *, FILE * );
void gds_eval_string( struct gds_Parser *, const char * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_INTERPRETER_H */

