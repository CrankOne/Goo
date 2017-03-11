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

# include <float.h>
# include <limits.h>
# include "goo_types.h"

# define init_atomic_dtype_introspection_code( num, cType, name, hdsnm ) \
    const TypeCode name ## _T_code = num;
    for_all_atomic_datatypes(init_atomic_dtype_introspection_code)
    # undef init_atomic_dtype_introspection_code

static struct TypeEntry {
    TypeCode code;
    const char name[16];
    uint8_t size;
} _atomicTypeNames[] = {
# define init_atomic_dtype_description( num, cType, name, hdsnm ) \
    {num, # name, sizeof(cType)},
    for_all_atomic_datatypes(init_atomic_dtype_description)
    # undef init_atomic_dtype_description
    {0, "unknown"},
};

const char *
binary_to_string( UByte b, char * bf ) {
    bf[8] = '\0';
    for( UByte i = 0; i < 8; ++i ) {
        bf[7-i] = b & ( 1 << i ) ? '1' : '0' ;
    }
    return bf;
}

const char *
get_atomic_type_name( TypeCode qc ) {
    struct TypeEntry * c = _atomicTypeNames;
    for( ; c->code; ++c ) {
        if( qc == c->code ) {
            return c->name;
        }
    }
    return c->name;
}

UByte
get_atomic_type_size( TypeCode qc ) {
    struct TypeEntry * c = _atomicTypeNames;
    for( ; c->code; ++c ) {
        if( qc == c->code ) {
            return c->size;
        }
    }
    return c->size;
}

