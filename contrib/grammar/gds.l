%option reentrant stack noyywrap bison-bridge bison-locations
%option extra-type="struct GDS_Parser *"
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
BP  (0b)
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]
TQ  \"{3}

%x COMMENT
%x INL_COMMENT
%x STRING_LIT
%s FORMULA

%{
# include "gds/goo_interpreter.h"
# include "gds_parser.generated.h"
# define MEMTOKEN(tc)                                                   \
    yyget_lval( yyscanner )->strval =                                   \
        gds_parser_replicate_token( yyget_extra(yyscanner), yytext );   \
    return tc;

#define YY_USER_ACTION                                              \
    yylloc->first_line   = yyget_lineno(yyscanner);                 \
    yylloc->first_column = yycolumn;                                \
    for(int i = 0; yytext[i] != '\0'; i++) {                        \
        if(yytext[i] == '\n') {                                     \
            yylloc->last_line++;                                    \
            yylloc->last_column = 0;                                \
        } else {                                                    \
            yylloc->last_column++;                                  \
        }                                                           \
    } {                                                             \
        struct GDS_Parser *pObjPtr =                                \
                (struct GDS_Parser *) yyget_extra(yyscanner);       \
        pObjPtr->location.first_line   = yylloc->first_line;        \
        pObjPtr->location.last_line    = yylloc->last_line;         \
        pObjPtr->location.first_column = yylloc->first_column;      \
        pObjPtr->location.last_column  = yylloc->last_column;       \
    }
/* FWD */
int comment(yyscan_t scanner);
int yyparse(struct GDS_Parser * P);
void yyerror( struct YYLTYPE * locp, struct GDS_Parser * P, const char * msg );
int lookup_symbol( struct GDS_Parser * P, const char * sym, YYSTYPE * );

void gds_error( struct GDS_Parser * P, const char * det );
void gds_warn( struct GDS_Parser * P, const char * det );
%}

%option nounput yylineno never-interactive

%%

"(#"            { BEGIN(COMMENT); }
<COMMENT>"#)"   { BEGIN(INITIAL); }
<COMMENT>[.\]]  { /* do nothing for commented out */ }
"#"             { BEGIN(INL_COMMENT); /* pass #-comment */ }
<INL_COMMENT>.  { /* do nothing for commented out */ }
<INL_COMMENT>\n { BEGIN(INITIAL); }

{WS}                                { /* whitespace separates tokens */ }

[Tt]rue|[Ee]nable|[Oo]n             { MEMTOKEN(T_TRUE); }
[Ff]alse|[Dd]isable|[Oo]ff          { MEMTOKEN(T_TRUE); }

{HP}{H}+{IS}?				        { MEMTOKEN(TI_HEX); }
{NZ}{D}*{IS}?				        { MEMTOKEN(TI_DEC); }
"0"{O}*{IS}?				        { MEMTOKEN(TI_OCT); }
{CP}?"'"([^'\\\n]|{ES})+"'"		    { MEMTOKEN(TI_ESC); }

{D}+{E}{FS}?				        { MEMTOKEN(TF_DEC); }
{D}*"."{D}+{E}?{FS}?			    { MEMTOKEN(TF_DEC); }
{D}+"."{E}?{FS}?			        { MEMTOKEN(TF_DEC); }
{HP}{H}+{P}{FS}?			        { MEMTOKEN(TF_HEX); }
{HP}{H}*"."{H}+{P}{FS}?			    { MEMTOKEN(TF_HEX); }
{HP}{H}+"."{P}{FS}?			        { MEMTOKEN(TF_HEX); }


{SP}?{TQ}                           { yy_push_state(STRING_LIT, yyscanner);
                                      gds_parser_str_lit( yyget_extra(yyscanner), yytext );
                                    }
<STRING_LIT>{
(.|\n)                              { gds_parser_append_lstr_lit( yyget_extra(yyscanner), yytext ); }
{TQ}                                { yyget_lval( yyscanner )->strval =
                                          gds_parser_opt_lstr_lit( yyget_extra(yyscanner) );
                                      yy_pop_state(yyscanner);
                                      return T_STRING_LITERAL; }
}

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+   { MEMTOKEN(T_STRING_LITERAL); }

{L}{A}*					            {   yyget_lval( yyscanner )->strval =
                                            gds_parser_replicate_token( yyget_extra(yyscanner), yytext );
                                        return lookup_symbol(yyget_extra(yyscanner), yytext, 0); }

";"                                 { gds_parser_free_buffer( yyget_extra(yyscanner) );
                                      return ';'; }
"|-"                                { return P_PIECEWISE_ELIF; }
"^-"                                { return P_PIECEWISE_FINALY; }
":"                                 { return ':'; }
"="                                 { return '='; }
"{"                                 { return '{'; }
"}"                                 { return '}'; }
"("                                 { return '('; }
")"                                 { return ')'; }
"/"                                 { return '/'; }
"."                                 { return '.'; }
"+"                                 { return '+'; }
"-"                                 { return '-'; }
"*"                                 { return '*'; }
"^"                                 { return '^'; }
"%"                                 { return '%'; }
"&"                                 { return '&'; }
"|"                                 { return '|'; }
"?"                                 { return '?'; }
">"                                 { return '>'; }
"<"                                 { return '<'; }

.                                   {   char bf[64];
                                        snprintf(bf, 64, "Uninpretable character '%c' had been met code.", yytext[0] );
                                        yyerror( yyget_lloc(yyscanner),
                                               yyget_extra(yyscanner),
                                               bf );
                                    }

%%

/*
 * Symbol lookup routine.
 */

int
lookup_symbol( struct GDS_Parser * P, const char * sym, YYSTYPE * yyst ) {
    /* TODO: we need here to provide mechanism for returning found
     *       symbol address. */
    return UNKNWN_SYM;
}

/*
 * Entry points
 */

void /* Evaluates a file */
gds_eval_file( struct GDS_Parser * P, FILE * inFile ) {
    if( yylex_init(&(P->scanner))) {
        fprintf(stderr, "Couldn't initialize scanner.\n");
        return;
    } else {
        yyset_extra( P, P->scanner );
        yyset_in( inFile, P->scanner );
        yyset_debug(1, P->scanner);
        if( yyparse( P ) ) {
            fprintf(stderr, "Parsing error.\n");
        }
    } yylex_destroy(P->scanner);
}

void /* Evaluates a string */
gds_eval_string( struct GDS_Parser * P, const char * gdsStr ) {
    # if 0
    int len = strlen( gdsStr );
    char * intermsyy = malloc(len+2);
    strcpy( intermsyy, gdsStr );
    intermsyy[ len + 1 ] = '\0';

    yyscan_t scanner;
    yylex_init(&scanner); {
        YY_BUFFER_STATE yyBf = yy_scan_string( intermsyy, scanner ); 
        yy_switch_to_buffer( yyBf, scanner );

        yyparse( scanner );

        yy_delete_buffer( yyBf, scanner );
    }yylex_destroy(scanner);

    free(intermsyy);
    # endif
}

