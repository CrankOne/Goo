/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
 * This function stores data in a C++ STL std::map
 * container. The data further can be retrieved by
 * \ref gds_get_error_description_for().
 *
 * The data assumed to be filled upon initialization, so no
 * concurrency-protection mechanisms are provided.
 */ void
gds_insert_error_description( void(*)(), int, const char * );

/**\func gds_insert_error_description
 * \brief Registers function pointer and its error description.
 *
 * This function retireves data from a C++ STL std::map
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

