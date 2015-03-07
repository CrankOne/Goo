%option reentrant stack noyywrap bison-bridge
D           [0-9]
L           [a-zA-Z_]
E           ([Ee][+-]?{D}+)
F           (({D}+.{D}*)|({D}*.{D}+))

%x COMMENT

%{
# include "gds/goo_interpreter.h"
# include "gds_parser.generated.h"
# define MEMTOKEN(tc) /*yyget_lval( yyscanner )->strval = yytext;*/ return tc;
/* FWD */
extern int yylex (YYSTYPE * yylval_param ,yyscan_t yyscanner);
int comment(yyscan_t scanner);
void count(yyscan_t scanner);
int yyparse(struct GDS_Parser * P);

%}

%option nounput

%%

"(#"            { BEGIN(COMMENT); }
<COMMENT>"#)"   { BEGIN(INITIAL); }
<COMMENT>.      { /* do nothing for commented out */ }

[ ]                                 { count(yyscanner); }
"#"                                 { comment(yyscanner); }

[Tt]rue|[Ee]nable|[Oo]n             { count(yyscanner); return T_TRUE; }
[Ff]alse|[Dd]isable|[Oo]ff          { count(yyscanner); return T_FALSE; }

-?([1-9]{D}*|{D})                   { MEMTOKEN(T_DEC); }
([1-9]{D}*|{D})u                    { MEMTOKEN(T_DEC_U); }
"0x"[a-fA-F0-9]+                    { MEMTOKEN(T_HEX); }
"0"[0-7]+                           { MEMTOKEN(T_OCT); }
"b"[01]+                            { MEMTOKEN(T_BIN); }

-?{F}                               { MEMTOKEN(T_FLOAT); }
-?({F}e-?{D}+)                      { MEMTOKEN(T_SCIFL); }
;

%%

int comment(yyscan_t scanner){
    char c;
    while ((c = input(scanner)) != '\n' && c != 0) {}
    return 0; /* TODO? */
}

void count(yyscan_t scanner){
    struct yyguts_t * yyg = (struct yyguts_t*) scanner;
    int i;
    for (i = 0; yytext[i] != '\0'; i++)
        if(yytext[i] == '\n')
            yycolumn = 0;
        else if (yytext[i] == '\t')
            yycolumn += 4 - (yycolumn % 4);
        else
            ++yycolumn;
    ECHO;
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
        yyset_in( inFile, P->scanner );
        yyset_debug(255, P->scanner);
        //yylex( P, P->scanner );
        if( yyparse( P->scanner ) ) {
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

