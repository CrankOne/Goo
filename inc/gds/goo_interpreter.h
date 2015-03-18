# ifndef H_GOO_INTERPRETER_H
# define H_GOO_INTERPRETER_H

# include "goo_config.h"
# include "goo_types.h"

# ifdef ENABLE_GDS

/**@file goo_interpreter.h
 * @brief File containing parser structure and its generic routines.
 *
 * Parser object stores all data associated with current GDS context.
 * Its routines manage intermediate data lifetime which are related
 * to corresponding generated lexer/parser routines.
 *
 * Namely, the parser struct:
 *  - Holds pool of tokens replicas. They're alive from the beginning
 *    of expression until the ';' terminal given.
 *  - Holds pool for long string literals. They're alive during whole
 *    parser lifetime.
 *  - Holds pool of parsed literals structures. They're alive during
 *    whole parser lifetime.
 *  - Hold a symbol tables, which are:
 *  -- Modules symbol tables(*), like `Math::` or `Units::`.
 *  -- Current session symbol table (in session-context -- variables
 *     and functions).
 *  -- Local symbol table that is cleared just after current function
 *     declaration ends.
 *
 * (*) -- each module's symbol table consists of variables and
 *        functions just as the current session symtable.
 *
 * Parser also keeps track the current lexical scanner instance and
 * current lexer's location. Pointer to current parser is stored
 * in FLEX/LEX `extra` section.
 */

# include <stdio.h>

# include "goo_gds_forwards.h"
# include "goo_literals.h"
# include "goo_variables.h"
# include "goo_functions.h"
# include "goo_expressions.h"

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

/**@struct gds_Hashtable
 * @brief A GDS hash table -- string-indexed dictionary of arbitrary data.
 *
 * Interpreted internally as C++ STL unordered map.
 */
typedef void * gds_Hashtable;

/** Allocates new hash table object. */
gds_Hashtable gds_hashtable_new();
/** Deletes allocated hash table object. */
void gds_hashtable_free(        gds_Hashtable );
/** Inserts new named data entry into hash table. */
void gds_hashtable_insert(      gds_Hashtable, const char *, void * );
/** Returns data associated with string key. NULL if not found. */
void * gds_hashtable_search(    gds_Hashtable, const char * );
/** Erases element by key. Raises `noSuchKey` if key is not found. */
void gds_hashtable_erase(       gds_Hashtable, const char * );

/**@brief GDS type.
 *
 * A type abstraction. Types aren't a part of GDS actual semantics,
 * but its instances can be treated as mathematical operands,
 * string constants, or objects for further using in module's
 * routines.
 */
struct gds_TypeID {
    uint16_t descriptor;
    void * typePtr;
};

/* TODO: type system. */

/**@struct gds_Module
 * @brief Represents a loaded module scope with all its symbols.
 *
 * Module can be presented as shared libraries or as preloaded GDS
 * scripts.
 */
struct gds_Module {
    char * name;
    gds_Hashtable functions,
                  variables,
                  types,
                  submodules;
};

/*
 * Parser object C-wrapper.
 */

# define for_all_parser_stacked_pools(m)    \
    m( ArgList,     1024        )           \
    m( VarList,     128*1024    )

# define for_all_parser_owned_pools(m)      \
    m( Literal,     256*1024    )           \
    m( Function,    256*1024    )           \
    m( Expr,        256*1024    )           \
    m( ExprList,    1024        )

struct gds_Parser {
    /* YACC/FLEX section {{{ */
    void * scanner;
    char currentFilename[128];
    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* }}} */

    struct ReplicaBuffer {
        char tokenReplicasBf[GDS_PARSER_EXPR_REPLICA_BUF_LENGTH];
        char * lastReplica;
        const char * tokenReplicasBfEnd;
    } buffers[GDS_PARSER_NSCOPES],
      * cScope;
    char * strLitBuffer,
         * strLitBufferC;

    # define declare_pool(typeName, size)       \
    struct Pool_ ## typeName {                  \
        struct gds_ ## typeName instns[size];   \
        uint32_t current;                       \
    } pool_ ## typeName;
    for_all_parser_stacked_pools(declare_pool)
    for_all_parser_owned_pools(declare_pool)
    # undef declare_pool

    struct gds_ArgList * argListChains[8];
    struct gds_ArgList ** currentLocArgListChain;

    struct gds_Module thisModule;
};

# define declare_pool_acq_routines(typeName, size)                             \
struct gds_ ## typeName * gds_parser_new_  ## typeName( struct gds_Parser * );
for_all_parser_stacked_pools(declare_pool_acq_routines)
for_all_parser_owned_pools(declare_pool_acq_routines)
# undef declare_pool_acq_routines

# define declare_pool_free_routines(typeName, size)                            \
void gds_parser_free_ ## typeName( struct gds_Parser * );
for_all_parser_stacked_pools(declare_pool_free_routines)
# undef declare_pool_free_routines

/** Sets current filename. Should be called before lexical analysis (yylex()). */
void gds_parser_set_filename( struct gds_Parser *,
                              const char * );

/** Creates new instance of GDS parser. */
struct gds_Parser * gds_parser_new();
/** Destroys a parser instance. */
void gds_parser_destroy( struct gds_Parser * );
/** Frees current parser tokens buffer. */
void gds_parser_free_buffer( struct gds_Parser * );

/*
 * Symbol table routines.
 */

/** Tries to resolve given identifier in module.
 *
 * @param[in] context       module to search in.
 * @param[in] identifier    key to search for.
 * @param[out] resut        will be set to 0, when identifier isn't found!
 *
 * @returns search result.
 * @retval  0 -- nothing found
 * @retval  1 -- submodule
 * @retval  2 -- variable
 * @retval  4 -- function
 * @retval  8 -- type
 * @retval -1 -- local variable (in f-n declaration)
 */
int8_t gds_parser_module_resolve_symbol( struct gds_Module * context,
                                         const char * identifier,
                                         void ** result );

/** Sets given variables dictionary as curent locvar dict keeping previous. */
void gds_parser_push_locvar_arglist( struct gds_Parser *, struct gds_ArgList * );
/** Denies current local variables dictionary and sets current to previous. */
void gds_parser_pop_locvar_arglist( struct gds_Parser * P );

/** Append this context's functions hash with given instance. */
struct gds_Expr * gds_parser_math_function_declare( struct gds_Parser *, struct gds_Function * );
/** Append this context's functions hash with given variable (any expression) instance. */
struct gds_Expr * gds_parser_variables_declare( struct gds_Parser *, struct gds_VarList * );

/*
 * Flex-related routines.
 * See scanner definition file for implementation.
 */

void gds_eval_file(   struct gds_Parser *, FILE * );
void gds_eval_string( struct gds_Parser *, const char * );

# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_INTERPRETER_H */

