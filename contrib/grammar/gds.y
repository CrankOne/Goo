%{
# include <assert.h>
# include <string.h>
# include "gds/goo_interpreter.h"
/* See:
 * http://lists.gnu.org/archive/html/bug-bison/2014-02/msg00002.html
 */

# define P_scanner (P->scanner)

/* FWD */
int yylex();
void yyerror();
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

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>        TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>        TF_DEC  TF_HEX

%token              T_STRING_LITERAL T_ID
%type<strval>       T_STRING_LITERAL T_ID

%type<value>        integral float numeric constexpr;
%type<mathExpr>     mathExpr;
%type<logical>      logicConst;

%start gdsExprLst

%%

/*
 * Basic expression
 */

gdsExprLst  : gdsExpr ';'               {}
            | gdsExprLst gdsExpr        {}
            ;

gdsExpr     : /* empty */               { /* do nothing */ }
            | constvalue                { no_side_effects($1); }
            | injectnDecl               { /* do nothing */ }
            | assgnmntDecl              { /* do nothing */ }
            ;

constvalue  : numeric                   {}
            | logic                     {}
            | T_STRING_LITERAL          {}
            | homognsArray              {}
            | heterognsArray            {}
            | 
            ;

injectnDecl : newID '<-' formula        { $$ = new_injection($1, $3); }
            ;

assgnmntDecl: newID '=' constvalue      { $$ = new_variable($1, $3); }
            ;


constval    : mathExpr                  { eval_math_expression(P, $1); }
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

integral    : TI_BIN                    { $$ = interpret_bin_integral(P, $1); }
            | TI_OCT                    { $$ = interpret_oct_integral(P, $1); }
            | TI_HEX                    { $$ = interpret_hex_integral(P, $1); }
            | TI_ESC                    { $$ = interpret_esc_integral(P, $1); }
            | TI_DEC                    { $$ = interpret_dec_integral(P, $1); }
            ;

float       : TF_DEC                    { $$ = interpret_float_dec(P, $1); }
            | TF_HEX                    { $$ = interpret_float_hex(P, $1); }
            ;

genertrExpr : '.' '.' integral          {}
            | integral '.' '.' integral {}
            | integral '.' '.' integral '.' '.' integral {}
            ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/goo_interpreter.h"

/* Special error function */
void gds_error( struct GDS_Parser * P, YYLTYPE * loc, const char * det ) {
    gds_parser_raise_error( P,
                            P->currentFilename,
                            loc->first_line,
                            loc->first_column,
                            loc->last_column,
                            det );
}

/* Special error function */
void gds_warn( struct GDS_Parser * P, YYLTYPE * loc, const char * det ) {
    gds_parser_warning( P,
                        P->currentFilename,
                        loc->first_line,
                        loc->first_column,
                        loc->last_column,
                        det );
}

void
yyerror( struct YYLTYPE * locp, struct GDS_Parser * P, const char * msg ) {
    gds_error( P, locp, msg );
}

