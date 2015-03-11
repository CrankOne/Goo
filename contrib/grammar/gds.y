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
         struct GDS_function * func;
                 const char ** idList;
                     uint8_t   logical;
           struct GDS_locvar * locvar;
}

%token              T_TRUE      T_FALSE
%token              P_ASSIGN    P_NAME      P_INJECTION  P_PIECEWISE_ELIF  P_PIECEWISE_FINALY

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>       TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL UNKNWN_SYM
%type<strval>       T_STRING_LITERAL UNKNWN_SYM

%token              T_LOCVAR
%type<locvar>       T_LOCVAR

%type<idList>       fVarList
%type<value>        numericCst nanVal integralCst floatCst;
%type<func>         fTail fDef;
%type<logical>      logicCst;

%left '-' '+'
%left '*' '/'
/*TODO: %precedence NEG   /* negation--unary minus */
%right '^' '%'       /* exponentiation */

%start gdsExprLst

%%

/*
 * Basic expression
 */

gdsExprLst  : /* empty */               {}
            | gdsExpr ';' gdsExprLst    {}
            ;

gdsExpr     : nanVal                    { /*no_side_effects_warning(P);*/ }
            | fDef                      { /* do nothing */ }
            | mathExpr                  {}
            ;

/*
 * Variables
 */

fDef        : UNKNWN_SYM '(' fVarList ')' P_INJECTION fTail { $$ = 0; /*function_set_name(P, $6, $1);*/ }
            ;

fVarList    : /* empty */               { $$ = 0; /*varlist_new(P, 0);*/           }
            | UNKNWN_SYM                { $$ = 0; /*varlist_new(P, $1);*/          }
            | fVarList ',' UNKNWN_SYM   { $$ = 0; /*varlist_append(P, $1, $3);*/   }
            ;

fTail       : mathExpr                  {}
            ;

/*
 * Binary arithmetical operations for numericals
 */

mathExpr    : mathOprnd                 {}
            | mathExpr '+' mathOprnd    {}
            | mathExpr '-' mathOprnd    {}
            | mathExpr '*' mathOprnd    {}
            | mathExpr '/' mathOprnd    {}
/*            | '-' mathExpr  %prec NEG   { negotiate(P, $2) }*/
            | mathExpr '^' mathOprnd    {}
            | mathExpr '%' mathOprnd    {}
            | '(' mathExpr ')'          {}
            ;

mathOprnd   : numericCst                {}
            | T_LOCVAR                  {}
            ;

/*
 * Basic values
 */

nanVal      : logicCst                  { $$ = $1; }
            | T_STRING_LITERAL          { $$ = $1; }
            ;

numericCst  : integralCst               { $$ = $1; }
            | floatCst                  { $$ = $1; }
            ;

integralCst : TI_BIN                    { $$ = interpret_bin_integral(P, $1); }
            | TI_OCT                    { $$ = interpret_oct_integral(P, $1); }
            | TI_HEX                    { $$ = interpret_hex_integral(P, $1); }
            | TI_ESC                    { $$ = interpret_esc_integral(P, $1); }
            | TI_DEC                    { $$ = interpret_dec_integral(P, $1); }
            ;

floatCst    : TF_DEC                    { $$ = interpret_float_dec(P, $1); }
            | TF_HEX                    { $$ = interpret_float_hex(P, $1); }
            ;

logicCst    : T_TRUE                    { $$ = 0xff; /*interpret_logic_true(P);*/ }
            | T_FALSE                   { $$ = 0x0; /*interpret_logic_false(P);*/ }
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

