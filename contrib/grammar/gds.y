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
%parse-param { struct gds_Parser * P}
%lex-param {   yyscan_t P_scanner}

%union {
                  const char * strval;
}

%token              T_TRUE      T_FALSE
%token              P_INJECTION P_ASSIGN    P_PIECEWISE_ELIF  P_PIECEWISE_ELSE
%token              P_GET       P_LET       P_EET       P_NET
%token              P_LAND      P_LXOR      P_LOR       P_BETWEEN

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>       TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL UNKNWN_SYM
%type<strval>       T_STRING_LITERAL UNKNWN_SYM

%left P_LAND
%left P_LOR P_LXOR
%left '!'

%left '-' '+'
%left '.'
%left '*' '/'
%right '^' '%'          /* exponentiation */

%start Scope

%%

/*
 * Scope
 */

Scope       : '{' ExprList '}'
            ;

ExprList    : Expr
            | ExprList ';' Expr
            ;

Expr        : FuncDecl
            | VarDecl
            ;

VarDecl     : UNKNWNID '=' RVal
            ;

FuncDecl    : FuncDclLVal '=' FuncTail
            ;

FuncDclLVal : UNKNWNID '(' ArgList ')'
            | UNKNWNID '[' '[' TensorIndexes ']' ']' '(' ArgList ')'
            ;

FuncTail    : MathFunc
            | PcwsFunc
            ;

PcwsFunc    : PcwsFunc P_PIECEWISE_ELIF PcwsPair
            | PcwsFunc P_PIECEWISE_ELSE MathFunc
            ;

PcwsPair    : LogicFunc '?' MathFunc
            ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/goo_interpreter.h"

void
yyerror( struct YYLTYPE * locp, struct gds_Parser * P, const char * msg ) {
    gds_error( P, msg );
}

