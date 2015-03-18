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
           struct gds_Module * module;

          struct gds_ArgList * argList;
          struct gds_VarList * varList;
         struct gds_ExprList * exprList;

                     uint8_t   locvarNo;
}

%token              T_TRUE      T_FALSE
%token              P_ASSIGN    P_NAME      P_INJECTION  P_PIECEWISE_ELIF  P_PIECEWISE_ELSE
%token              P_GET       P_LET       P_EET       P_NET
%token              P_LAND      P_LXOR      P_LOR

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<strval>       TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<strval>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL UNKNWN_SYM
%type<strval>       T_STRING_LITERAL UNKNWN_SYM

%token              TYPEID DCLRD_FUN DCLRD_VAR DCLRD_MDLE LOCVAR


%type<GTID>         TYPEID
%type<module>       DCLRD_MDLE
%type<locvarNo>     LOCVAR

%type<value>        DCLRD_VAR logicCst numericCst vaLit integralCst floatCst;
%type<func>         DCLRD_FUN fDecl fDef mathExpr mathOprnd funcTail;
%type<expr>         expr rvalExpr manifest

%type<argList>      fArgList
%type<varList>      varHead varDecl unknwSymLst
%type<exprList>     varTail gdsExprLst

%left P_LAND
%left P_LOR P_XOR
%left '!'

%left '-' '+'
%left '.'
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

manifest    : fDef                      { $$ = gds_expr_from_func_decl( P, $1 ); }
            | varDecl                   { $$ = gds_expr_from_var_decls( P, $1 ); }
            ;

rvalExpr    : vaLit                     { $$ = gds_expression_from_literal( $1 ); }
            | mathExpr                  { $$ = gds_expression_from_math_expr( $1 ); }
            | '(' manifest ')'          { $$ = $2; }
            ;

rvalExprLst : rvalExpr                  { /*TODO*/ }
            | rvalExpr ',' rvalExprLst  { /*TODO*/ }
            ;

/*
 * Variables declaration
 */

varDecl     : varHead "=" varTail       { $$ = gds_variable_init_list(P, $1, $3); }
            ;

varHead     : TYPEID unknwSymLst        {$$ = gds_variable_spec_type_for(P, $1, $2);}
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

fDef        : fDecl P_INJECTION funcTail
                { $1->content.asFunction.f = $3;
                  gds_parser_pop_locvar_arglist( P );
                  gds_parser_deepcopy_function( P, $1 );
                  $$ = $1; }
            ;

funcTail    : mathExpr                  {}
            | piecewsLst                {}
            ;

piecewsLst  : piecewsFrg                                    {}
            | piecewsLst ',' P_PIECEWISE_ELIF piecewsFrg    {}
            | piecewsLst ',' P_PIECEWISE_ELSE mathExpr      {}
            ;

piecewsFrg  : logicExpr '?' mathExpr    {}
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

funcRfrnc   : DCLRD_FUN '(' ')'
                { /*TODO*/ }
            | DCLRD_FUN '(' rvalExprLst ')'
                { /*TODO*/ }
            ;

/*
 * Logical expressions
 */

logicExpr   : logicAtom                          {}
            | logicAtom P_LAND logicExpr         {}
            | logicAtom P_LOR  logicExpr         {}
            | logicAtom P_LXOR logicExpr         {}
            | '!' logicExpr %prec P_LXOR         {}
            ;

logicAtom   : mathExpr                           {}
            | mathExpr '>' mathExpr              {}
            | mathExpr '<' mathExpr              {}
            | mathExpr P_GET mathExpr            {}
            | mathExpr P_LET mathExpr            {}
            | mathExpr P_EET mathExpr            {}
            | mathExpr P_NET mathExpr            {}
            | mathExpr '~' mathExpr '~' mathExpr {}
            /* Ternary (interval) logic */
            | mathExpr '<' mathExpr '<' mathExpr
                {}
            | mathExpr '>' mathExpr '>' mathExpr
                {}
            | mathExpr '<' mathExpr P_LET mathExpr
                {}
            | mathExpr P_LET mathExpr '<' mathExpr
                {}
            | mathExpr P_LET mathExpr P_LET mathExpr
                {}
            | mathExpr P_GET mathExpr '>' mathExpr
                {}
            | mathExpr '>' mathExpr P_GET mathExpr
                {}
            | mathExpr P_GET mathExpr P_GET mathExpr
                {}
            ;

/*
 * Binary arithmetical operations for numericals
 */

mathExpr    : mathOprnd                 { $$ = $1; }
            | mathExpr '+' mathExpr     { $$ = gds_math(P, '+', $1, $3); }
            | mathExpr '-' mathExpr     { $$ = gds_math(P, '-', $1, $3); }
            | mathExpr '.' mathExpr     { $$ = gds_math(P, '.', $1, $3); }
            | mathExpr '*' mathExpr     { $$ = gds_math(P, '*', $1, $3); }
            | mathExpr '/' mathExpr     { $$ = gds_math(P, '/', $1, $3); }
            | '-' mathExpr  %prec '*'   { $$ = gds_math_negotiate(P, $2); }
            | mathExpr '^' mathExpr     { $$ = gds_math(P, '^', $1, $3); }
            | mathExpr '%' mathExpr     { $$ = gds_math(P, '%', $1, $3); }
            | '(' mathExpr ')'          { $$ = $2; }
            ;

mathOprnd   : numericCst                { $$ = gds_math_new_func_from_const(  P, $1 ); }
            | funcRfrnc                 { /*TODO*/ }
            | LOCVAR                    { $$ = gds_math_new_func_from_locvar( P, $1 ); }
            | DCLRD_VAR                 { /*TODO*/ }
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

