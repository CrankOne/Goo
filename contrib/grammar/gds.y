%{
# include <assert.h>
# include <string.h>
# include "gds/interpreter.h"
# include "gds/expression.h"

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
   struct ArithmeticConstant * arithLit;
}

%token              T_TRUE      T_FALSE
%token              P_INJECTION P_ASSIGN    P_PIECEWISE_ELIF    P_PIECEWISE_ELSE
%token              P_GET       P_LET       P_EET               P_NET
%token              P_LAND      P_LXOR      P_LOR               P_BETWEEN
%token              P_NOT
%token              P_FOR       P_IS
%token              P_CTX_MOD_OPEN P_CTX_MOD_CLOSE

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>       TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL UNKNWN_SYM
%type<strval>       T_STRING_LITERAL UNKNWN_SYM


%type<arithLit> numerical integral floatingPoint;


%left P_LAND
%left P_LOR P_LXOR
%left '!'

%left '-' '+'
%left '.'
%left '*' '/'
%right '^' '%'          /* exponentiation */

%start expression

%%

     expression : numerical  { /*$$ = gds_expression_from_literal(P, $1); TODO */ }
                ;

      numerical : integral      { $$ = $1; }
                | floatingPoint { $$ = $1; }
                ; 

       integral : TI_BIN    {
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_integral_literal, $$, $1, 2 );
                            }
                | TI_OCT    {
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_integral_literal, $$, $1, 8 );
                            }
                | TI_HEX    {
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_integral_literal, $$, $1, 16 );
                            }
                | TI_ESC    {   $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_esc_integral, $$, $1 );
                            }
                | TI_DEC    {
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_integral_literal, $$, $1, 10 );
                            }
                ;

  floatingPoint : TF_DEC    { 
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_float_literal, $$, $1, 10 );
                            }
                | TF_HEX    {   
                                $$ = gds_parser_new_ArithmeticConstant(P);
                                RIF( gds_interpr_float_literal, $$, $1, 16 );
                            }
                ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/interpreter.h"

void
yyerror( struct YYLTYPE * locp, struct gds_Parser * P, const char * msg ) {
    gds_error( P, msg );
}

