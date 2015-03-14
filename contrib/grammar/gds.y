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
          struct gds_Literal * value;
         struct gds_Function * func;
           struct gds_TypeID * GTID;
             struct gds_Expr * expr;

          struct gds_ArgList * argList;
          struct gds_VarList * varList;
         struct gds_ExprList * exprList;
}

%token              T_TRUE      T_FALSE
%token              P_ASSIGN    P_NAME      P_INJECTION  P_PIECEWISE_ELIF  P_PIECEWISE_FINALY

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>       TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL UNKNWN_SYM
%type<strval>       T_STRING_LITERAL UNKNWN_SYM

%token              TYPEID
%type<GTID>         TYPEID


%type<value>        logicCst numericCst vaLit integralCst floatCst;
%type<func>         fDecl fDef mathExpr mathOprnd;
%type<expr>         expr rvalExpr manifest

%type<argList>      fArgList
%type<varList>      varHead varDecl unknwSymLst
%type<exprList>     varTail gdsExprLst

%left '-' '+'
%left '*' '/'
/*TODO: %precedence NEG   /* negation--unary minus */
%right '^' '%'       /* exponentiation */

%start gdsExprLst

%%

/*
 * Basic expression
 */

gdsExprLst  : /* empty */               { $$ = gds_parser_new_ExprList(P);
                                          $$->next = NULL;
                                          $$->cexpr = NULL; }
            | expr ';' gdsExprLst       { $3->next = gds_parser_new_ExprList(P);
                                          $3->next->next = NULL;
                                          $3->next->cexpr = $1;
                                          $$ = $3; }
            ;

expr        : rvalExpr                  { $$ = $1; }
            | manifest                  { $$ = $1; }
            ;

manifest    : fDef                      { $$ = gds_parser_math_function_declare( P, $1 ); }
            | varDecl                   { $$ = gds_parser_variables_declare( P, $1 ); }
            ;

rvalExpr    : vaLit                     { $$ = $1; }
            | mathExpr                  { $$ = $1; }
            | '(' manifest ')'          { $$ = $2; }
            ;

/*
 * Variables declaration
 */

varDecl     : varHead "=" varTail       { $$ = gds_variable_init_list(P, $1, $3); }
            ;

varHead     : TYPEID unknwSymLst        {$$ = gds_variable_spec_type_for(P, $2, $1);}
            | unknwSymLst               {$$ = $1;}
            ;

unknwSymLst : UNKNWN_SYM
                { $$ = gds_parser_new_VarList(P);
                  $$->identifier = $1;
                  $$->next = NULL; }
            | unknwSymLst ',' UNKNWN_SYM
                { $1->next = gds_parser_new_VarList(P);
                  $1->next->identifier = $3;
                  $1->next->next = NULL;
                  $$ = $1; }
            ;

varTail     : rvalExpr
                { $$ = gds_parser_new_ExprList(P);
                  $$->cexpr = $1;
                  $$->next = NULL; }
            | varTail ',' rvalExpr
                { $$ = $1;
                  $$->next = gds_parser_new_ExprList(P);
                  $$->next->cexpr = $3;
                  $$->next->next = NULL; }
            ;

/*
 * Functions
 */

fDecl       : UNKNWN_SYM '(' fArgList ')'
                { $$ = gds_parser_new_Function( P );
                  gds_math_function_init( P, $$, NULL, $1, $3 );
                  gds_parser_push_locvar_arglist( P, $3 );
                }
            ;

fDef        : fDecl P_INJECTION mathExpr
                { $1->content.asFunction.f = $3;
                  gds_math_function_resolve( P, $1 );
                  gds_parser_pop_locvar_arglist( P );
                  $$ = $1; }
            ;

fArgList    : /* empty */               
                { struct gds_ArgList * al = gds_parser_new_ArgList(P);
                  al->identifier = NULL;
                  al->next = NULL;
                  $$ = al; }
            | UNKNWN_SYM
                { struct gds_ArgList * al = gds_parser_new_ArgList(P);
                  al->identifier = $1;
                  al->next = NULL;
                  $$ = al; }
            | fArgList ',' UNKNWN_SYM   { $$ = $1;
                                          $$->next = gds_parser_new_ArgList(P);
                                          $$->next->identifier = $3;
                                          $$->next->next = NULL; }
            ;

/*
 * Binary arithmetical operations for numericals
 */

mathExpr    : mathOprnd                 { $$ = $1; }
            | mathExpr '+' mathExpr     { $$ = gds_math(P, '+', $1, $3); }
            | mathExpr '-' mathExpr     { $$ = gds_math(P, '-', $1, $3); }
            | mathExpr '*' mathExpr     { $$ = gds_math(P, '*', $1, $3); }
            | mathExpr '/' mathExpr     { $$ = gds_math(P, '/', $1, $3); }
            | '-' mathExpr  %prec '*'   { $$ = gds_math_negotiate(P, $2); }
            | mathExpr '^' mathExpr     { $$ = gds_math(P, '^', $1, $3); }
            | mathExpr '%' mathExpr     { $$ = gds_math(P, '%', $1, $3); }
            | '(' mathExpr ')'          { $$ = $2; }
            ;

mathOprnd   : numericCst                { $$ = gds_math_new_func_from_const(  P, $1 ); }
            /*TODO| UNKNWN_SYM                { $$ = gds_math_new_func_from_locvar( P, $1 ); }*/
            ;

/*
 * Basic values
 */

vaLit       : logicCst                  { $$ = $1; }
            | T_STRING_LITERAL          { $$ = interpret_string_literal( P, $1 ); }
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

logicCst    : T_TRUE                    { $$ = interpret_logic_true(P); }
            | T_FALSE                   { $$ = interpret_logic_false(P); }
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

