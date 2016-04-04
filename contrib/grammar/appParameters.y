%{
# include <assert.h>
# include <string.h>
//#include "..."

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

%union {
                  const char * strval;
                     Logical   logicalValue;
                   LIntegral   integralValue;
                      LFloat   floatingPointValue;
               struct Option * option;
           /*struct OptionPair {
                                struct Option * option;
                                   const char * strval;
                                      Logical   logicalValue;
                                    LIntegral   integralValue;
                                       LFloat   floatingPointValue;
                             }; XXX?*/
       struct ShortCdOptions * scdOptions;
}

%token              T_TRUE      T_FALSE
%type<Logical>      Logical

%token              TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC
%type<LIntegral>    TI_BIN  TI_OCT  TI_HEX  TI_ESC  TI_DEC

%token              TF_DEC  TF_HEX
%type<LFloat>       TF_DEC  TF_HEX

%token              T_STRING_LITERAL
%type<strval>       T_STRING_LITERAL

%token              T_OPTION
%type<option>       T_OPTION

%token              T_SHORT_OPTIONS_CONCATENATED
%token<scdOptions>  T_SHORT_OPTIONS_CONCATENATED


%type<arithLit> numerical integral floatingPoint;

%start cmdLArgument

%%

   cmdLArgument : argument      { /* TODO: push back positional */ }
                | option        { /* TODO: set appropriate CMDL-paser FSM state */ }
                | optionWvalue  { /* TODO: validate and set option value */ }
                | options       { /* TODO: combined behaviour of all above */ }
                ;

        options : T_DASH T_DASH T_OPTION_ID {}
                ;

         option : T_OPTION      { $$ = $1; }
                ;

       argument : logical       {}
                | numerical     {}
                | T_STRING_LITERAL {}
                ;

        logical : T_TRUE        {}
                | T_FALSE       {}
                ;

      numerical : integral      { $$ = $1; }
                | floatingPoint { $$ = $1; }
                ; 

       integral : TI_BIN        {}
                | TI_OCT        {}
                | TI_HEX        {}
                | TI_ESC        {}
                | TI_DEC        {}
                ;

  floatingPoint : TF_DEC        {}
                | TF_HEX        {}
                ;

%%
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "gds/interpreter.h"

void
yyerror( struct YYLTYPE * locp, const char * msg ) {
    appp_error( P, msg );
}

