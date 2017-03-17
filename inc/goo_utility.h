/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_GOO_UTILITY_H
# define H_GOO_UTILITY_H

# include "goo_types.h"

# include <stddef.h>

# ifndef STRINGIFY_MACRO_ARG
# define STRINGIFY_MACRO_ARG(a) _STRINGIFY_MACRO_ARG(a)
# define _STRINGIFY_MACRO_ARG(a) #a
# endif  // STRINGIFY_MACRO_ARG

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

/**@brief raises custom GOO-exception from c-code
 *
 * C-function with C++ linkage that throws Goo-exception.
 * Not defined for C++ code.
 *
 * Note: implemented at goo_exception.cpp
 */
int goo_C_error( ErrCode, const char * fmt, ... ) __attribute__ ((noreturn));

# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_UTILITY_H */

