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
# include "goo_ansi_escseq.h"

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

/**\enum GDS_DiagnosticStatus
 * \brief This values is used by \ref gds_diagnostic_message() to indicate message type.
 * \see gds_diagnostic_message()
 *
 * TODO: move it outta here
 */
enum GDS_DiagnosticStatus {
    gds_kDiagnostic_warning = 0x2,
    gds_kDiagnostic_error = 0x3,
    /* ... */
};

/**\func gds_diagnostic_message
 * \brief A function providing messaging while lexer/parser/evaluator is active.
 *
 * Acts like printf()-family accepting same format chars + some useful shortcuts,
 * enclosed in `${}`:
 *   * `${location}` corresponds to current input position. Depending on file/stdin
 * is used, can hold input filename.
 * TODO: we'll probable need additional shortcuts.
 *
 * \param p  a GDS parser instanc
 * TODO: doc
 * TODO: move it outta here
 */
void gds_diagnostic_message(
        struct gds_Parser * p,
        enum GDS_DiagnosticStatus status,
        const char * fmt, ... );

/**\func gds_abort
 * \brief Interrupts current parser sesion returning code provided in
 * second argument.
 *
 * TODO: doc
 * TODO: move it outta here
 * */
void gds_abort( struct gds_Parser *, int );

/**\def gds_treat_function_result
 * \brief A shortcut for routing between \ref gds_error_printf
 * / \ref gds_wrnng_printf variadic-arguments function.
 *
 * When rc parameter is 0, mothing will happen. Otherwise, depending
 * on rc warning/error treatment mechanism will be invoked.
 */
# define gds_treat_function_result( f, rc )     \
if(rc) {                                        \
    if( rc < 0 ) {                              \
        gds_diagnostic_message( P,              \
                gds_kDiagnostic_error,          \
"${location}:" ESC_BLDRED "error" ESC_CLRCLEAR ": invokation of %s(...) issued: %s",    \
                # f,                            \
                gds_get_error_description_for(  \
                        (void(*)()) f, rc ) );  \
        gds_abort(P, -1);                       \
    } else if( rc > 0 ) {                       \
        gds_diagnostic_message( P,              \
                gds_kDiagnostic_warning,        \
"${location}:" ESC_BLDYELLOW "warning" ESC_CLRCLEAR ": invokation of %s(...) issued: %s",    \
                # f,                            \
                gds_get_error_description_for(  \
                        (void(*)()) f, rc ) );  \
    }                                           \
}

/**\def RIF 
 * \brief An error-handling macro for dealing with \ref gds_Parser
 * error tables.
 *
 * This macro wraps the specific function call. It invokes function
 * provided in first argument with other macro parameters as its
 * function arguments and stores result. When result is not equal
 * to zero, it will treat the result with \ref gds_treat_function_result().
 *
 * \see gds_treat_function_result()
 * \see gds_Parser
 * \see gds_ParserRoutineResultTable
 * */
# define RIF( function_name, ... )                  \
{ int rc = function_name( __VA_ARGS__ );            \
  gds_treat_function_result(function_name, rc);     }


struct ArithmeticConstant * gds_parser_new_ArithmeticConstant( struct gds_Parser * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */
# endif  /* H_GOO_INTERPRETER_H */
