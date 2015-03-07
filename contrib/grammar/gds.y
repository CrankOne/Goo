%{

extern int yylex();
extern char yytext[];

# include <assert.h>
# include <string.h>
# include "gds/goo_interpreter.h"

struct YYLTYPE;

void yyerror( struct YYLTYPE *, struct HDSParser *, const char * msg );

%}

%pure-parser
 /*%name-prefix="gds"*/
%locations
%defines
%error-verbose

 /*%define api.pure full*/
%parse-param {struct HDSParser * pw}

%union {
                  const char * strval;
              struct GDS_Num * numeric;
            struct GDS_mexpr * mathExpr;
             struct GDS_expr * manifestation;
}

%token              T_TRUE  T_FALSE

%token              T_DEC   T_DEC_U T_HEX   T_OCT   T_BIN
%type<strval>       T_DEC   T_DEC_U T_HEX   T_OCT   T_BIN

%token              T_FLOAT         T_SCIFL
%type<strval>       T_FLOAT         T_SCIFL

%token              T_ID            T_STRING
%type<strval>       T_ID            T_STRING

%type<numeric>      integral float numeric;
%type<mathExpr>     mathExpr;

%start manifestation

%%

manifestation: /*empty*/                { $$ = 0; }
            | mathExpr                  { $$ = eval_math_expression($1); }
            /* ... */
            ;

integral    : T_DEC                     { $$ = interpret_dec($1);   }
            | T_DEC_U                   { $$ = interpret_dec_u($1); }
            | T_hex                     { $$ = interpret_hex($1);   }
            | T_OCT                     { $$ = interpret_oct($1);   }
            | T_BIN                     { $$ = interpret_bin($1);   }
            ;

float       : T_FLOAT                   { $$ = interpret_float($1); }
            | T_SCIFL                   { $$ = interpret_scifl($1); }
            ;

numeric     : integral                  { $$ = $1; }
            | float                     { $$ = $1; }
            ;

mathExpr    : numeric                   { $$ = $1; }
            ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/goo_routines.h"

extern char yytext[];
extern int column;

void
yyerror( struct YYLTYPE * locp, struct GDSParser * pw, const char * msg ) {
    fprintf( stderr, "\nParser: %s\n", msg );
    /*HDS_error( interpreter_ec, "Parser: %s", mes );*/
}

