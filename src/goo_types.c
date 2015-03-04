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

static Size _atomicTypeSizes[] = {
# define init_atomic_dtype_sz( num, cType, name, hdsnm ) \
    sizeof(name),
    for_all_atomic_datatypes(init_atomic_dtype_sz)
    # undef init_atomic_dtype_sz
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

