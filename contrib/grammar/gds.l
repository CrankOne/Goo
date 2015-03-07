%option reentrant stack noyywrap bison-bridge
D           [0-9]
L           [a-zA-Z_]
E           ([Ee][+-]?{D}+)
F           (({D}+.{D}*)|({D}*.{D}+))

%{
# include "gds/goo_interpreter.h"
# include "../generated/goo_parser.h"
void count();
int comment();
extern int yylex();
# define MEMTOKEN(tc) yyget_lval( yyscanner )->strval = memtok( yytext ); return tc;
%}

%option nounput

%%

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
 /*
{L}({L}|{D}|"\\ ")*                 { count(); yylval.strval = strdup( yytext); return T_ID;  }
L?\"(\\.|[^\\"])*\"                 { count(); yylval.strval = strdup( yytext); return T_STRING; }
"("                                 { count(); return '('; }
")"                                 { count(); return ')'; }
"{"                                 { count(); return '{'; }
"}"                                 { count(); return '}'; }
","                                 { count(); return ','; }
":"                                 { count(); return ':'; }
"["                                 { count(); return '['; }
"]"                                 { count(); return ']'; }
"<"                                 { count(); return '<'; }
">"                                 { count(); return '>'; }
 */;

%%

int comment(yyscan_t scanner){
    char c;
    while ((c = input(scanner)) != '\n' && c != 0) {}
        //putchar(c);
    return 0; /* TODO? */
}

struct yyguts_t;
int yyparse( struct HDSParser * );

void count(yyscan_t scanner){
    struct yyguts_t * yyg = (struct yyguts_t*) scanner;
    /* TODO */
    ECHO;
}

struct hds_Decl *
hds_parse_str( struct HDSParser * pw,
               const char * hdsStr ) {
    int len = strlen(hdsStr);
    char * intermsyy = malloc(len+2);
    strcpy( intermsyy, hdsStr );
    intermsyy[ len + 1 ] = '\0';

    yyscan_t scanner;
    yylex_init(&scanner);

    YY_BUFFER_STATE yyBf = yy_scan_string( intermsyy, scanner ); 
    yy_switch_to_buffer( yyBf, scanner );

    yyparse(pw);

    yy_delete_buffer( yyBf, scanner );
    yylex_destroy(scanner);

    free(intermsyy);
    return 0;  /* TODO */
}


