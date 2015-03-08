%option reentrant stack noyywrap bison-bridge bison-locations
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

%x COMMENT

%{
# include "gds/goo_interpreter.h"
# include "gds_parser.generated.h"
/* TODO: we should use here kind a reentrant buffer or whatever
 * instead strdup it each time we need memorize a particular token.
 * Probably, best way is to supplement scanner with some custom data
 * structure via `YYEXTRA_TYPE yyextra_r;`.
 */
# define MEMTOKEN(tc)                                   \
    printf(">%s<\n", yytext);                           \
    yyget_lval( yyscanner )->strval = strdup(yytext);   \
    return tc;
/* FWD */
int comment(yyscan_t scanner);
int yyparse(struct GDS_Parser * P);

%}

%option nounput

%%

"(#"            { BEGIN(COMMENT); }
<COMMENT>"#)"   { BEGIN(INITIAL); }
<COMMENT>.      { /* do nothing for commented out */ }

{WS}                                { /* whitespace separates tokens */ }
"#".*                               { /* pass #-comment */ }

[Tt]rue|[Ee]nable|[Oo]n             { MEMTOKEN(T_TRUE); }
[Ff]alse|[Dd]isable|[Oo]ff          { MEMTOKEN(T_TRUE); }

{HP}{H}+{IS}?				        { MEMTOKEN(I_CONSTANT); }
{NZ}{D}*{IS}?				        { MEMTOKEN(I_CONSTANT); }
"0"{O}*{IS}?				        { MEMTOKEN(I_CONSTANT); }
{CP}?"'"([^'\\\n]|{ES})+"'"		    { MEMTOKEN(I_CONSTANT); /*interpret escape symbols*/ }

{D}+{E}{FS}?				        { MEMTOKEN(F_CONSTANT); }
{D}*"."{D}+{E}?{FS}?			    { MEMTOKEN(F_CONSTANT); }
{D}+"."{E}?{FS}?			        { MEMTOKEN(F_CONSTANT); }
{HP}{H}+{P}{FS}?			        { MEMTOKEN(F_CONSTANT); }
{HP}{H}*"."{H}+{P}{FS}?			    { MEMTOKEN(F_CONSTANT); }
{HP}{H}+"."{P}{FS}?			        { MEMTOKEN(F_CONSTANT); }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ MEMTOKEN(STRING_LITERAL); }
{L}{A}*					            { MEMTOKEN(T_ID); }

";"                                 { MEMTOKEN(';'); }
"="                                 { MEMTOKEN('='); }

%%

/*
 * Entry points
 */

void /* Evaluates a file */
gds_eval_file( struct GDS_Parser * P, FILE * inFile ) {
    if( yylex_init(&(P->scanner))) {
        fprintf(stderr, "Couldn't initialize scanner.\n");
        return;
    } else {
        yyset_in( inFile, P->scanner );
        yyset_debug(255, P->scanner);
        //yylex( P, P->scanner );
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

