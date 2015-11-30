%option reentrant stack noyywrap bison-bridge bison-locations
%option extra-type="struct gds_Parser *"
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

%x COMMENT
%x INL_COMMENT
%x STRING_LIT
%s FORMULA

%{
# include "gds/interpreter.h"
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
        struct gds_Parser *pObjPtr =                                \
                (struct gds_Parser *) yyget_extra(yyscanner);       \
        pObjPtr->location.first_line   = yylloc->first_line;        \
        pObjPtr->location.last_line    = yylloc->last_line;         \
        pObjPtr->location.first_column = yylloc->first_column;      \
        pObjPtr->location.last_column  = yylloc->last_column;       \
    }
/* FWD */
int comment(yyscan_t scanner);
int yyparse(struct gds_Parser * P);
void yyerror( struct YYLTYPE * locp, struct gds_Parser * P, const char * msg );
int lookup_symbol( struct gds_Parser * P, const char * sym );

void gds_error( struct gds_Parser * P, const char * det );
void gds_warn( struct gds_Parser * P, const char * det );
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

    /* COMPOSITE NOTATION SYMBOLS */

"<-"                                { return P_INJECTION; }
"|-"                                { return P_PIECEWISE_ELIF; }
"elif"                              { return P_PIECEWISE_ELIF; }
"^-"                                { return P_PIECEWISE_ELSE; }
"else"                              { return P_PIECEWISE_ELSE; }

    /* LOGICAL OPERATORS */

">="                                { return P_GET; }
"<="                                { return P_LET; }
"=="                                { return P_EET; /*exactly equals to*/ }
"~~"                                { return P_NET; /*nearly equals to*/ }
"&&"                                { return P_LAND; }
"and"                               { return P_LAND; }
"||"                                { return P_LOR; }
"or"                                { return P_LOR; }
"^^"                                { return P_LXOR; }
"xor"                               { return P_LXOR; }
"not"                               { return P_NOT; }
".."                                { return P_BETWEEN; }

    /* LITERALS */

[Tt]rue|[Ee]nable|[Oo]n|[Yy]es      { MEMTOKEN(T_TRUE); }
[Ff]alse|[Dd]isable|[Oo]ff|[Nn]o    { MEMTOKEN(T_FALSE); }

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


    /* MULTILINE STRING LITERALS */

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

    /* IDENTIFIERS */

(({L}{A}{0,127})::)*{L}{A}{0,127}   {   int rt;
                                        yyget_lval( yyscanner )->strval =
                                            gds_parser_replicate_token( yyget_extra(yyscanner), yytext );
                                        rt = lookup_symbol(yyget_extra(yyscanner), yytext );
                                        return rt;
                                    }

    /* CONTEXT MANIPULATION */

"/:"                                { return P_CTX_MOD_OPEN; }
":/"                                { return P_CTX_MOD_CLOSE; }
"for"                               { return P_FOR; }
"is"                                { return P_IS; }

    /* SIMPLE GRAMMAR TERMINALS */

";"                                 { gds_parser_free_buffer( yyget_extra(yyscanner) );
                                      return ';'; }
">"                                 { return '>'; }
"<"                                 { return '<'; }
":"                                 { return ':'; }
"="                                 { return P_ASSIGN; }
"{"                                 { return '{'; }
"}"                                 { return '}'; }
"("                                 { return '('; }
")"                                 { return ')'; }
"/"                                 { return '/'; }
"."                                 { return '.'; }
","                                 { return ','; }
"+"                                 { return '+'; }
"-"                                 { return '-'; }
"*"                                 { return '*'; }
"^"                                 { return '^'; }
"%"                                 { return '%'; }
"&"                                 { return '&'; }
"|"                                 { return '|'; }
"?"                                 { return '?'; }
"~"                                 { return '~'; }
"["                                 { return '['; }
"]"                                 { return ']'; }

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

/**@function lookup_symbol
 * @brief Provides search among current session hashtables.
 *
 * This routine resolves named identifier in current context, sets the
 * appropriate YYSTYPE's lval union field and returns terminal code.
 * When symbol isn't found, lval stays unchanged.
 *
 * @param[in] P         current parser (context)
 * @param[in] sym       C-string containing null-teminated identifier str.
 * @param[out] lval     Scanner's lval instance to be affilated with.
 * @returns appropriate terminal code, or UNKNWN_SYM, if no symbols found. 
 */
int
lookup_symbol( struct gds_Parser * P, const char * sym ) {
    # if 0
    int8_t rt = 0,
           wasScopeResolution = 0;
    const char * c = sym;
    char   cSymName[128] = {'\0'},
         * cDest;

    struct gds_Module * cModule = &(P->thisModule);

    do {
        for( cDest = cSymName;
             '\0' != *c;
             c++ ) {
            if( ':' == *c && ':' == *(++c) ) {
                *(++cDest) = '\0';
                wasScopeResolution = 1;
                printf( " - \"%s\" /\n", cSymName );  /*XXX*/
                break;
            }
            *(cDest++) = *c;
        }
        {  /* Try to resolve symbol as function, variable, or submodule.
            * All non-module names aren't allowed here, the point is
            * to figure out shadowing naming.
            */
            void * result;
            rt = gds_parser_module_resolve_symbol( cModule,
                                                   cSymName,
                                                  &result );
            if( rt > 0x1 && &(P->thisModule) != cModule ) {
                /* Here, we aren't in current module's context and we got
                 * an unkown symbol, that means something goes wrong.
                 */
                char bf[2560];
                if( rt > 0x1 ) {
                    snprintf(bf, 2560, "Symbol \"%s\" in identifier \"%s\" refers to non-module.",
                             cSymName, sym);
                    gds_error(P, bf);
                } else if( !rt ) {
                    snprintf(bf, 2560, "Identifier \"%s\" can not be resolved due to unkown symbol \"%s\".",
                             sym, cSymName);
                    gds_error(P, bf);
                }
            }
        }
    } while( '\0' != *c );

    if( !wasScopeResolution && P->currentLocArgListChain ) {
        /* Now, try to resolve symbol as locvar, if we're in function declaration. */
        const union gds_ArgList * c = *(P->currentLocArgListChain);
        uint8_t lvno = 0;
        if( c->head.first ) {
            c = c->head.first;
            do {
                if( !strcmp( c->entry.this_, cSymName ) ) {
                    yyget_lval( P->scanner )->locvarNo = lvno;
                    /* printf( "LEXER: sym \"%s\" resolved as locvar #%d.\n",
                            cSymName, (int) lvno ); XXX */
                    return LOCVAR;
                }
                lvno++;
                c = c->entry.next;
            } while( c );
        }
    }

    void * result;
    rt = gds_parser_module_resolve_symbol(
                 cModule,
                 cSymName,
                &result );
    switch( rt ) {
        case 0x8 : { /* type */
            yyget_lval( P->scanner )->GTID = result;
            printf("LEXER: \"%s\" resolved as a type.\n", sym);
            return TYPEID;
        } break;
        case 0x4 : { /* function */
            yyget_lval( P->scanner )->func = result;
            printf("LEXER: \"%s\" resolved as a function.\n", sym);
            return DCLRD_FUN;
        } break;
        case 0x2 : { /* variable */
            yyget_lval( P->scanner )->value = result;
            printf("LEXER: \"%s\" resolved as a variable.\n", sym);
            return DCLRD_VAR;
        } break;
        case 0x1 : { /* submodule */
            yyget_lval( P->scanner )->module = result;
            printf("LEXER: \"%s\" resolved as a module name.\n", sym);
            return DCLRD_MDLE;
        } break;
        case 0:
        default: { /* Symbol isn't known */
            printf("LEXER: \"%s\" is unresolved.\n", sym);
            return UNKNWN_SYM;
        }
    };
    # else
    return 0;
    # endif
}

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

