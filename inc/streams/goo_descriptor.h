# ifndef H_GOO_STREAMING_SERIES_DESCRIPTOR_H
# define H_GOO_STREAMING_SERIES_DESCRIPTOR_H

# include "types.h"

# define for_all_serialized_entity_bitflags(m)  \
    m( 1,   uniform,        "uniform type size",    "variadic size type"    )   \
    m( 2,   arithmetic,     "arithmetic",           "non-arithmetic"        )   \
    m( 2,   hinted,         "has typeinfo hint",    NULL                    )   \
    /* ... */

union goo_streaming_Descriptor {
    UByte asArithmetic;
    UByte asUniformEntity;
    UByte asVariadicEntity;
};


void goo_streaming_descriptor_set_uniform_flag(       union goo_streaming_Descriptor * D, TypeCode  );
void goo_streaming_descriptor_set_uniform_size(       union goo_streaming_Descriptor * D, Size );
Size goo_streaming_descriptor_get_uniform_size( const union goo_streaming_Descriptor D );

    void goo_streaming_descriptor_set_arithmetic_flag(            union goo_streaming_Descriptor * D, TypeCode );
    void goo_streaming_descriptor_set_arithmetic_type_code(       union goo_streaming_Descriptor * D, TypeCode );
TypeCode goo_streaming_descriptor_get_arithmetic_type_code( const union goo_streaming_Descriptor D );

# endif  /* H_GOO_STREAMING_SERIES_DESCRIPTOR_H */

