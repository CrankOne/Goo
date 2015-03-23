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
           struct gds_Module * module;
          struct gds_Literal * value;
         struct gds_Function * func;
        struct gds_LFunction * lfunc;
           struct gds_TypeID * GTID;
             struct gds_Expr * expr;
            struct gds_Range * range;
             struct gds_Pair * pair;
              struct gds_Arr * array;
           struct gds_AscArr * ascArray;
          struct gds_PcwsTrm * pieceTerm;

           union gds_ArgList * argList;
           union gds_VarList * varList;
          union gds_ExprList * exprList;
          union gds_PairList * pairList;
       union gds_PcwsTrmList * pcwsList;

                     uint8_t   locvarNo;
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

%token              TYPEID DCLRD_FUN DCLRD_VAR DCLRD_MDLE LOCVAR


%type<GTID>         TYPEID
%type<module>       DCLRD_MDLE
%type<locvarNo>     LOCVAR

%type<value>        DCLRD_VAR logicCst numericCst vaLit integralCst floatCst funcRfrnc;
%type<func>         DCLRD_FUN fDecl fDef mathExpr mathOprnd funcTail;
%type<range>        range
%type<array>        posArray

%type<pair>         ascPair
%type<pairList>     ascPairsLst
%type<ascArray>     ascArray

%type<lfunc>        logicAtom logicExpr
%type<expr>         expr rvalExpr manifest

%type<argList>      fArgList
%type<varList>      varHead varDecl unknwSymLst
%type<pieceTerm>    piecewsFrg
%type<pcwsList>     piecewsLst

%type<exprList>     varTail gdsExprLst rvalExprLst

%left P_LAND
%left P_LOR P_LXOR
%left '!'

%left '-' '+'
%left '.'
%left '*' '/'
/*TODO: %precedence NEG   /* negation--unary minus */
%right '^' '%'          /* exponentiation */

%start gdsExprLst

%%

/*
 * Basic expression
 */

gdsExprLst  : /* empty */               { $$ = gds_parser_new_ExprList(P); }
            | expr ';' gdsExprLst       { gds_ExprList_append(P, $$ = $3, $1); }
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
            | ascArray                  { /*TODO*/ }
            ;

rvalExprLst : rvalExpr                  { $$ = gds_parser_new_ExprList(P); }
            | rvalExpr ',' rvalExprLst  { gds_ExprList_append(P, $$ = $3, $1); }
            ;

/*
 * Variables declaration
 */

varDecl     : varHead P_ASSIGN varTail  { $$ = gds_variable_init_list(P, $1, $3); }
            ;

varHead     : TYPEID unknwSymLst        {$$ = gds_variable_spec_type_for(P, $1, $2);}
            | unknwSymLst               {$$ = $1;}
            ;

unknwSymLst : UNKNWN_SYM
                { $$ = gds_parser_new_VarList(P);
                  gds_VarList_append( P, $$, $1 ); }
            | unknwSymLst ',' UNKNWN_SYM
                { gds_VarList_append(P, $$ = $1, $3); } 
            ;

varTail     : rvalExpr
                { $$ = gds_parser_new_ExprList(P);
                  gds_ExprList_append( P, $$, $1 ); }
            | varTail ',' rvalExpr
                { gds_ExprList_append( P, $$ = $1, $3 ); }
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

funcTail    : mathExpr                  { $$ = $1; }
            | piecewsLst                { $$ = gds_math_piecewise_unite($1); }
            ;

piecewsLst  : piecewsFrg
                { $$ = gds_parser_new_PcwsTrmList(P);
                  gds_PcwsTrmList_append(P, $$, $1);  }
            | piecewsLst ',' P_PIECEWISE_ELIF piecewsFrg
                { gds_PcwsTrmList_append(P, '|', $$ = $1, $4); }
            | piecewsLst ',' P_PIECEWISE_ELSE mathExpr
                { gds_PcwsTrmList_append(P, '~', $$ = $1, $4); }
            ;

piecewsFrg  : logicExpr '?' mathExpr    { $$ = gds_math_piecewise_term_new(P, $1, $3); }
            ;

fArgList    : /* empty */               
                { $$ = gds_parser_new_ArgList(P); }
            | UNKNWN_SYM
                { $$ = gds_parser_new_ArgList(P);
                  gds_ArgList_append(P, $$, $1); }
            | fArgList ',' UNKNWN_SYM
                { gds_ArgList_append(P, $$ = $1, $3); }
            ;

funcRfrnc   : DCLRD_FUN '(' ')'
                { $$ = gds_math_substitute_function( NULL ); }
            | DCLRD_FUN '(' rvalExprLst ')'
                { $$ = gds_math_substitute_function( $3 ); }
            ;

/*
 * Logical expressions
 */

logicExpr   : logicAtom                          { $$ = $1; }
            | logicAtom P_LAND logicExpr         { $$ = gds_logic_bin( P, 1, $1, '&', $3 ); }
            | logicAtom P_LOR  logicExpr         { $$ = gds_logic_bin( P, 1, $1, '|', $3 ); }
            | logicAtom P_LXOR logicExpr         { $$ = gds_logic_bin( P, 1, $1, '^', $3 ); }
            | '!' logicExpr %prec P_LXOR         { $$ = gds_logic_unary( P, 1, $2, '!' ); }
            ;

logicAtom   : mathExpr                           { $$ = gds_logic_from_math(P, $1); }
            | mathExpr '>' mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_greater, $3 ); }
            | mathExpr '<' mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_less,    $3 ); }
            | mathExpr P_GET mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_greaterOrEquals, $3 ); }
            | mathExpr P_LET mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_lessOrEquals, $3 ); }
            | mathExpr P_EET mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_exactEquals, $3 ); }
            | mathExpr P_NET mathExpr
                { $$ = gds_logic_math( P, $1, gds_math_binary_l_nearlyEquals, $3 ); }
            /* Ternary (interval) logic */
            | mathExpr '~' mathExpr '~' mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_binary_l_nearlyEquals ); }
            | mathExpr '<' mathExpr '<' mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_ltlt ); }
            | mathExpr '>' mathExpr '>' mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_gtgt ); }
            | mathExpr '<' mathExpr P_LET mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_ltle ); }
            | mathExpr P_LET mathExpr '<' mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_lelt ); }
            | mathExpr P_LET mathExpr P_LET mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_lele ); }
            | mathExpr P_GET mathExpr '>' mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_gegt ); }
            | mathExpr '>' mathExpr P_GET mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_gtge ); }
            | mathExpr P_GET mathExpr P_GET mathExpr
                { $$ = gds_logic_ternary_math( P, $1, $3, $5, gds_math_tl_gege ); }
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

mathOprnd   : numericCst                { $$ = gds_math_new_func_from_const(  P, $1 );  }
            | funcRfrnc                 { $$ = $1; }
            | LOCVAR                    { $$ = gds_math_new_func_from_locvar( P, $1 );  }
            | DCLRD_VAR                 { $$ = gds_math_new_func_from_var(P, $1);       }
            | range                     { $$ = gds_math_new_func_from_range(P, $1);     }
            | posArray                  { $$ = gds_math_new_func_from_array(P, $1); }
            | mathOprnd '[' mathExpr ']'
                { $$ = gds_array_subset($1, $3); }
            ;

range       : '{' mathExpr P_BETWEEN mathExpr '}'
                { $$ = gds_range_new( $2, 1, $4 ); }
            | '{' mathExpr P_BETWEEN mathExpr P_BETWEEN mathExpr '}'
                { $$ = gds_range_new( $2, $4, $6 ); }
            ;

/*
 * Positional and associative arrays
 */

posArray    : '{' rvalExprLst '}'       { $$ = $2; }
            ;

ascArray    : '{' ascPairsLst '}'       { $$ = gds_asc_array_new(P, $2); }
            ;

ascPairsLst :                           { $$ = NULL; }
            | ascPair ',' ascPairsLst   { $$ = gds_asc_array_append(P,  $3, $1 ); }
            ;

ascPair     : UNKNWN_SYM ':' rvalExpr   { $$ = gds_asc_array_pair_new(P, $1, $3); }
            ;

/*
 * Basic literals
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

