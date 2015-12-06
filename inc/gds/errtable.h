/**\file errtable.h
 * \biref GDS parser-specific error tables.
 *
 * This declares structure of a global pool containing gds_* functions result
 * explaination.
 */
# ifndef H_GOO_GDS_ROUTINES_ERROR_TABLE_H
# define H_GOO_GDS_ROUTINES_ERROR_TABLE_H

# include "goo_config.h"
# include "goo_types.h"

# ifdef ENABLE_GDS
# ifdef __cplusplus
extern "C" {
# endif  /* __cplusplus */

/**\func gds_insert_error_description
 * \brief Registers function pointer and its error description.
 *
 * This function stores data in a C++ STL std::unordered_map
 * container. The data further can be retrieved by
 * \ref gds_get_error_description_for().
 *
 * The data assumed to be read-only, so no concurrency-protection
 * mechanisms are provided.
 */ void
gds_insert_error_description( void(*)(), int, const char * );

/**\func gds_insert_error_description
 * \brief Registers function pointer and its error description.
 *
 * This function retireves data from a C++ STL std::unordered_map
 * container. This data is usually inserted with
 * \ref gds_insert_error_description().
 *
 * The data assumed to be read-only, so no concurrency-protection
 * mechanisms are provided.
 */ const char *
gds_get_error_description_for( void(*)(), int );


struct gds_FunctionResultCodeEntry {
    int code;
    const char explaination[256];
};


# define gds_declare_result_table_for_function( functionName )       \
static struct gds_FunctionResultCodeEntry functionName ## _returns[] =

# define gds_declare_expl_registry                     \
static void _static_register_result_tables_for_ ## moduleName () __attribute__((constructor(156))); \
void _static_register_result_tables_for_ ## moduleName ()

# define gds_register_error_explainations_for( functionName )                                       \
    for( nFunction = 0;                                                                             \
         nFunction < sizeof(functionName ## _returns)/sizeof(struct gds_FunctionResultCodeEntry);   \
         nFunction++ ) { gds_insert_error_description( (void(*)()) functionName,                    \
                                                functionName ## _returns[nFunction].code,           \
                                                functionName ## _returns[nFunction].explaination ); }
    

# ifdef __cplusplus
}
# endif  /* __cplusplus */
# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_ROUTINES_ERROR_TABLE_H */

