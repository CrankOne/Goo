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
          struct gds_Literal * literal;
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


%type<literal>      literal numerical integral floatingPoint logic string;


%left P_LAND
%left P_LOR P_LXOR
%left '!'

%left '-' '+'
%left '.'
%left '*' '/'
%right '^' '%'          /* exponentiation */

%start expression

%%

     expression : literal   { /*$$ = gds_expression_from_literal(P, $1); TODO */ }
                ;

        literal : logic         { $$ = $1; }
                | numerical     { $$ = $1; }
                | string        { $$ = $1; }
                ;

      numerical : integral      { $$ = $1; }
                | floatingPoint { $$ = $1; }
                ; 

       integral : TI_BIN    { $$ = interpret_bin_integral(P, $1); }
                | TI_OCT    { $$ = interpret_oct_integral(P, $1); }
                | TI_HEX    { $$ = interpret_hex_integral(P, $1); }
                | TI_ESC    { $$ = interpret_esc_integral(P, $1); }
                | TI_DEC    { $$ = interpret_dec_integral(P, $1); }
                ;

  floatingPoint : TF_DEC    { $$ = interpret_float_dec(P, $1); }
                | TF_HEX    { $$ = interpret_float_hex(P, $1); }
                ;

         string : T_STRING_LITERAL { $$ = interpret_string_literal( P, $1 ); }
                ;

          logic : T_TRUE    { $$ = interpret_logic_true(P); }
                | T_FALSE   { $$ = interpret_logic_false(P); }
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

