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

%token              T_TRUE      T_FALSE
%token              P_ASSIGN    P_NAME      P_INJECTION

%token              T_I_CONST T_F_CONST T_STRING_LITERAL T_ID
%type<strval>       T_I_CONST T_F_CONST T_STRING_LITERAL T_ID

%type<value>        integral float numeric constexpr;
%type<mathExpr>     mathExpr;
%type<logical>      logicConst;

%start expression

%%

expression  : /* empty */               {}
            | manifest ';' expression   {}
            ;

manifest    : constexpr                 { empty_manifest( P, $1 ); }
            | T_ID '=' constexpr        { declare_named_constant(P, $1, $3); }
            | funcDecl P_INJECTION expression { /*declare_math_function($1, $3);*/ }
            ;

funcDecl    : T_ID '(' argList ')'      { /*$$ = new_function( $1, $3 );*/ }
            ;

argList     : T_ID                      {}
            | T_ID ',' argList          {}
            ;

constexpr   : mathExpr                  { eval_math_expression(P, $1); }
            | T_STRING_LITERAL          { memorize_string_literal(P, $1); }
            ;

numeric     : integral                  { $$ = $1; }
            | float                     { $$ = $1; }
            ;

logicConst  : T_TRUE                    { $$ = 0xFF; }
            | T_FALSE                   { $$ = 0x0; }

mathExpr    : numeric                   { $$ = mexpr_from_constant(P, $1); }
            | logicConst                { $$ = mexpr_from_logic(P, $1); }
            ;

integral    : T_I_CONST                 { $$ = interpret_integral(P, $1);   }
            ;

float       : T_F_CONST                 { $$ = interpret_float(P, $1); }
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

