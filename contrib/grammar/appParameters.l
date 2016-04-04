%option reentrant stack noyywrap bison-bridge bison-locations
%option extra-type="struct appp_Parser *"
O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_\\]
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
HYPHEN -

%{
/* See: http://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html */
//# include "app_parameters.h"
# define MEMTOKEN(tc)                                                   \
    yyget_lval( yyscanner )->strval =                                   \
        gds_lexer_replicate_token( yyget_extra(yyscanner), yytext );    \
    /* TODO: check, if double-hyphen was given */                       \
    return tc;

/* FWD */
int yyparse(struct gds_Parser * P);
void yyerror( struct YYLTYPE * locp, struct gds_Parser * P, const char * msg );
/*int lookup_symbol( struct gds_Parser * P, const char * sym );*/
%}

%option nounput yylineno never-interactive

%%

    /* LITERALS */

[Tt]rue|[Ee]nable|[Oo]n|[Yy]es      { MEMTOKEN(T_TRUE); }
[Ff]alse|[Dd]isable|[Oo]ff|[Nn]o    { MEMTOKEN(T_FALSE); }
{HYPHEN}                            { MEMTOKEN(T_STDSTREAM); }
{HYPHEN}{HYPHEN}                    { MEMTOKEN(T_OPTEND); }

"b"[01]+{IS}?                       { MEMTOKEN(TI_BIN); }
{HP}{H}+{IS}?				        { MEMTOKEN(TI_HEX); }
{NZ}{D}*{IS}?				        { MEMTOKEN(TI_DEC); }
"0"{O}*{IS}?				        { MEMTOKEN(TI_OCT); }
{CP}?"'"([^'\\\n]|{ES})+"'"		    { MEMTOKEN(TI_ESC); }

{D}+{E}{FS}?				        { MEMTOKEN(TF_DEC); }
{D}*"."{D}+{E}?{FS}?			    { MEMTOKEN(TF_DEC); }
{D}+"."/[^"."]{E}?{FS}?			    { MEMTOKEN(TF_DEC); }
{HP}{H}+{P}{FS}?			        { MEMTOKEN(TF_HEX); }
{HP}{H}*"."{H}+{P}{FS}?			    { MEMTOKEN(TF_HEX); }
{HP}{H}+"."{P}{FS}?			        { MEMTOKEN(TF_HEX); }

    /* OPTIONS */
({HYPHEN}{A}{1,1024})                {  /* TODO: check, if double-hyphen was given */
                                        /* TODO: process composite string
                                         * using options description available and decompose it
                                         * to:
                                         *      T_OPTION                        -v
                                         *      T_OPTION_ARGUMENT_PAIR          -v3
                                         *      T_SHORT_OPTIONS_CONCATENATED    -vo  / -vo3
                                         *
                                         */
                                    }
({HYPHEN}{HYPHEN}{A}{1,1024})       {   /* TODO: check, if double-hyphen was given */
                                        /* TODO: process composite string
                                         * using options description available and decompose it
                                         * to:
                                         *      T_OPTION                        --verbose
                                         *      T_OPTION_ARGUMENT_PAIR          --verbose=3
                                         *
                                         */
                                    }

    /* CONSIDER OTHER AS JUST A STRING LITERAL WITH NO SPECIAL MEANING */
.                                   { MEMTOKEN( T_STRING_LITERAL ); }

%%

/*
 * Entry points
 */

void /* Evaluates a file */
gds_eval_file( struct gds_Parser * P, FILE * inFile ) {
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
gds_eval_string( struct gds_Parser * P, const char * gdsStr ) {
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

