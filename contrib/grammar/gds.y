%{
# include <assert.h>
# include <string.h>
# include "gds/goo_interpreter.h"
/* See:
 * http://lists.gnu.org/archive/html/bug-bison/2014-02/msg00002.html
 */
# define P_scanner (P->scanner)
%}
%debug
 /*%define api.pure full*/
%pure-parser
%locations
%defines
%error-verbose
%parse-param { struct GDS_Parser * P}
%lex-param {   yyscan_t P_scanner}

%union {
                  const char * strval;
            struct GDS_Value * value;
            struct GDS_mexpr * mathExpr;
                     uint8_t   logical;
}

%token              T_TRUE  T_FALSE

%token              I_CONSTANT F_CONSTANT STRING_LITERAL T_ID
%type<strval>       I_CONSTANT F_CONSTANT STRING_LITERAL T_ID

%type<value>        integral float numeric constexpr;
%type<mathExpr>     mathExpr;
%type<logical>      logicConst;

%start expression

%%

expression  : /* empty */               {}
            | expression ';'            {}
            | manifest ';' expression   {}
            ;

manifest    : constexpr                 { empty_manifest( $1 ); }
            | T_ID '=' constexpr        { declare_named_constant($1, $3); }
            ;

constexpr   : mathExpr                  { eval_math_expression($1); }
            | STRING_LITERAL            { memorize_string_literal($1); }
            ;

numeric     : integral                  { $$ = $1; }
            | float                     { $$ = $1; }
            ;

logicConst  : T_TRUE                    { $$ = 0xFF; }
            | T_FALSE                   { $$ = 0x0; }

mathExpr    : numeric                   { $$ = mexpr_from_constant($1); }
            | logicConst                { $$ = mexpr_from_logic($1); }
            ;

integral    : I_CONSTANT                { $$ = interpret_integral($1);   }
            ;

float       : F_CONSTANT                { $$ = interpret_float($1); }
            ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/goo_interpreter.h"

void
yyerror( struct YYLTYPE * locp, struct GDS_Parser * pw, const char * msg ) {
    fprintf( stderr, "\nParser: %s\n", msg );
    /*HDS_error( interpreter_ec, "Parser: %s", mes );*/
}

