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

# include "arithmetic_eval.h"

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct gds_Parser;

/** \note implemented in lexer file */
void gds_error( struct gds_Parser * P, const char * det );

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
    /* YACC/FLEX section {{{ */
    void * scanner;
    char currentFilename[128];
    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* }}} */
};

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */
# endif  /* H_GOO_INTERPRETER_H */
