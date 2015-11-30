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

/*# include "goo_gds_forwards.h"*/
# include "literals.h"
# include "expression.h"

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

/** \note implemented in lexer file */
void gds_error( struct gds_Parser * P, const char * det );

/**@typedef gds_Hashtable
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
/** Replaces named data entry in hash table with given one. Checks existency. */
void gds_hashtable_replace(     gds_Hashtable, const char *, void * );
/** Returns data associated with string key. NULL if not found. */
void * gds_hashtable_search(    gds_Hashtable, const char * );
/** Erases element by key. Raises `noSuchKey` if key is not found. */
void gds_hashtable_erase(       gds_Hashtable, const char * );


/**@typedef gds_Heap
 * @brief A container for managing dynamic heap-allocated memory.
 *
 * Internally represented jast as unordered_set of malloc()'d pointers.
 */
typedef void * gds_Heap;

/** Allocates new heap abstraction. */
gds_Heap gds_heap_new();
/** Deletes heap and all its blocks. */
void gds_heap_free( gds_Heap );
/** Allocates a memory block. */
void * gds_heap_alloc( gds_Heap, uint32_t blocklen );
/** Frees a memory block. */
void gds_heap_erase( gds_Heap, void * chunk );

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
 * Lists X-macros
 */

# define for_all_parser_lists(m)                            \
    m( Arg,     const char *,               1024 )          \
    m( Var,     const char *,           128*1024 )          \
    m( Expr,    struct gds_Expr *,      128*1024 )          \
    m( PcwsTrm, struct gds_PcwsTrm *,   128      )          \
    /* ... */

/* Declare list union */

# define gds_list_declare_lstruct( StructName, StructPtrType, l )   \
    union gds_ ## StructName ## List {                              \
        struct StructName ## Entry {                                \
            union gds_ ## StructName ## List * next;                \
            StructPtrType this_;                                    \
        } entry;                                                    \
        struct StructName ## Head {                                 \
            union gds_ ## StructName ## List * first, * last;       \
        } head; };
for_all_parser_lists( gds_list_declare_lstruct );
# undef gds_list_declare_lstruct

# define gds_list_declare_routines(StructName, StructPtrType, l)    \
union gds_ ## StructName ## List *                                  \
    gds_ ## StructName ## List_append(                              \
            struct gds_Parser *,                                    \
            union gds_ ## StructName ## List *,                     \
            StructPtrType );
for_all_parser_lists(gds_list_declare_routines);
# undef gds_list_declare_routines

/*
 * Pools X-macro
 */

# define for_all_parser_owned_pools(m)              \
    m( struct, Literal,     256*1024    )

/*
 * Parser
 */

struct gds_Parser {
    /* YACC/FLEX section {{{ */
    void * scanner;
    char currentFilename[128];
    struct Location {
        uint32_t first_line,    last_line,
                 first_column,  last_column;
    } location;
    /* }}} */

    /* Tokens replication {{{ */
    struct ReplicaBuffer {
        char tokenReplicasBf[GDS_PARSER_EXPR_REPLICA_BUF_LENGTH];
        char * lastReplica;
        const char * tokenReplicasBfEnd;
    } buffers[GDS_PARSER_NSCOPES],
      * cScope;
    char * strLitBuffer,
         * strLitBufferC;
    /* }}} */

    /* Argument lists (locvars) managing {{{ */
    union gds_ArgList * argListChains[8];
    union gds_ArgList ** currentLocArgListChain;
    /*}}}*/

    /*Moduels subsystem {{{*/
    struct gds_Module thisModule;
    /*}}}*/

    /* Lists pools {{{ */
    # define declare_lists_pool( StructName, StructPtrType, l ) \
    struct Pool_ ## StructName ## List {                        \
        union gds_ ## StructName ## List instns[l];             \
        uint32_t current;                                       \
    } pool_ ## StructName ## List;
    for_all_parser_lists(declare_lists_pool);
    # undef gds_list_declare_lstruct
    /* }}} */

    /* Entities pools {{{ */
    # define declare_pool(T, typeName, size)            \
    T Pool_ ## typeName {                               \
        T gds_ ## typeName instns[size];                \
        uint32_t current;                               \
    } pool_ ## typeName;
    for_all_parser_owned_pools(declare_pool)
    # undef declare_pool
    /* }}} */

    gds_Heap literals;
};

# define declare_pool_acq_routines(T, typeName, size)                           \
T gds_ ## typeName * gds_parser_new_  ## typeName( struct gds_Parser * );
for_all_parser_owned_pools(declare_pool_acq_routines)
# undef declare_pool_acq_routines

# define declare_pool_acq_routines(StructName, StructPtrType, l)                \
union gds_ ## StructName ## List *                                              \
gds_parser_new_ ## StructName ## List( struct gds_Parser * );                   \
union gds_ ## typeName ## List * gds_parser_new_  ## typeName( struct gds_Parser * );
for_all_parser_lists(declare_pool_acq_routines)
# undef declare_pool_acq_routines

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
void gds_parser_push_locvar_arglist( struct gds_Parser *, union gds_ArgList * );
/** Denies current local variables dictionary and sets current to previous. */
void gds_parser_pop_locvar_arglist( struct gds_Parser * P );

# if 0
/** Produces pools-unrelated deep copy of a function. */
struct gds_Function * gds_parser_deepcopy_function( struct gds_Parser * P, struct gds_Function * f );
# endif

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

