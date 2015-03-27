# ifndef H_GOO_GDS_FORWARDS_H
# define H_GOO_GDS_FORWARDS_H

struct gds_Parser;
struct gds_Literal;
struct gds_TypeID;
struct gds_Expr;
struct gds_Formula;
struct gds_Arr;
struct gds_Range;
struct gds_Pair;

union gds_ExprList;
union gds_VarList;
union gds_PcwsTrmList;
union gds_PairList;

/*
 * Those functions are necessary for other
 * routines to work with parser.
 *
 */

/** Creates a copy of token string using internal static memory pool. */
char * gds_parser_replicate_token( struct gds_Parser *, const char * );

/* Re-inits string literal buffer. */
void gds_parser_str_lit( struct gds_Parser *, const char * );

/* Appends string in string literal buffer with given character. */
void gds_parser_append_lstr_lit( struct gds_Parser *, const char * );

/* Returns string literal bufer begin. */
char * gds_parser_opt_lstr_lit( struct gds_Parser * );



/** Acquires void anonymous gds_Function object from parser's pool. */
struct gds_Function * gds_parser_math_function_new( struct gds_Parser * );

/*
 * Error
 */

# ifdef __cplusplus
extern "C" {
# endif

void gds_error(
        struct gds_Parser * P,
        const char * details );

void gds_warn(
        struct gds_Parser * P,
        const char * details );

# ifdef __cplusplus
}  // extern "C"
# endif

# endif  /* H_GOO_GDS_FORWARDS_H */

