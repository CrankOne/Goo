# ifndef H_GOO_UTILITY_H
# define H_GOO_UTILITY_H

# include "goo_types.h"

# define STRINGIFY_MACRO_ARG(a) _STRINGIFY_MACRO_ARG(a)
# define _STRINGIFY_MACRO_ARG(a) #a

# ifdef __cplusplus
extern "C" {
# endif

/**\brief Generates a triangle-distributed numbers with given width(?). */
double goo_dstr_triangular( const double c );

/**\brief Returns current timestamp. Uses internal static buffer. */
const char * hctime();

/**\brief Returns fancy text timestamp. Uses internal static buffer. */
const char * get_timestamp();

/**\brief Quick factorial up to 12. */
unsigned long factorial_16( unsigned char );

/**\brief Writes rounded memory size up to Tb into buffer and returns buffer. */
char *
rounded_mem_size( unsigned long toPrint,
                  char * buffer,
                  unsigned char bufLength );

/**\brief Writes fancy-formatted size string.
 *
 * Produces result in form of:
 *   `[N-Tb?]\`[N-Gb?]\`[N-Mb?]\`[N-kb?]\`[N-b]`
 * Where N-xx is a number of terrabytes/gigabytes/... that is contained
 * by provided size argument number. It is not a decimal number, just a
 * convinient way to approximately estimate actual size.
 */
char *
fancy_mem_size( unsigned long toPrint,
                  char * buffer,
                  unsigned char bufLength );

/**\brief  A static buffer (uses own) version of rounded_mem_size(). */
char * rounded_mem_size_stb( unsigned long toPrint );

/**\brief A static buffer (uses own) version of fancy_mem_size(). */
char * fancy_mem_size_stb( unsigned long toPrint );


/**\brief Replaces pattern in string without heap (re)allocation. */
int replace_string( const char * sourceString,
                    const char * origToken,
                    const char * replacement,
                    char * buffer,
                    const size_t bufferLength);


# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_UTILITY_H */

