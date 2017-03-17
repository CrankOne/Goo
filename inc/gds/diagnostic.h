/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/***/
# ifndef H_GDS_GOO_INTERPRETER_DIAGNOSTIC_H
# define H_GDS_GOO_INTERPRETER_DIAGNOSTIC_H
# include "goo_config.h"
# ifdef ENABLE_GDS
# include "gds/errtable.h"
# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

struct gds_Parser;

/**\note implemented in goo_types.cpp */
void gds_error( struct gds_Parser * P, const char * det );

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
 * */
void gds_abort( struct gds_Parser *, int );

/**\def gds_treat_function_result
 * \brief A shortcut for routing between \ref gds_error_printf
 * / \ref gds_wrnng_printf variadic-arguments function.
 *
 * When rc parameter is 0, nothing will happen. Otherwise, depending
 * on rc warning/error treatment mechanism will be invoked.
 */
# define gds_treat_function_result( f, rc )     \
if(rc) {                                        \
    if( rc < 0 ) {                              \
        gds_diagnostic_message( P,              \
                gds_kDiagnostic_error,          \
"${location}:" ESC_BLDRED "error" ESC_CLRCLEAR ": invokation of %s(...) issued: %s", \
                # f,                            \
                gds_get_error_description_for(  \
                        (void(*)()) f, rc ) );  \
        gds_abort(P, -1);                       \
    } else if( rc > 0 ) {                       \
        gds_diagnostic_message( P,              \
                gds_kDiagnostic_warning,        \
"${location}:" ESC_BLDYELLOW "warning" ESC_CLRCLEAR ": invokation of %s(...) issued: %s", \
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

# ifdef __cplusplus
}
# endif /* __cplusplus */
# endif  /* ENABLE_GDS */
# endif  /* H_GDS_GOO_INTERPRETER_DIAGNOSTIC_H */

