# ifndef H_GOO_STREAMING_GOO_REFLECTION_H
# define H_GOO_STREAMING_GOO_REFLECTION_H

# include "goo_types.h"


/*
 *      uvwx ????
 *      u -- use next byte?
 *      v -- is this a variadic? uniform otherwise
 *      w -- is this an array?
 *      x -- when it is an uniform, this bit should be considered as arithmetic indicator
 *
 *      ?000 ????
 *      single uniform, non-arithmetic type (UniformStorable) instance, probably with
 *      additional data.
 *
 *      ?001 ????
 *      single arithmetic
 *
 *      ?010 ????
 *      array of non-arithmetic uniforms
 *
 *      ?011 ????
 *      array of arithmetics
 *
 *      ?100 ????
 *      single variadic storable instance
 *
 *      ?101 ????
 *      RESERVED
 *
 *      ?110 ????
 *      Array of variadic-length storables
 *
 *      ?111 ????
 *      RESERVED
 */

# define for_each_goo_reflection_serialized_flags(m) \
    m( useNextByte,      0 ) \
    m( isVariadic,       1 ) \
    m( isArray,          2 ) \
    m( isArithmetic,     3 ) \
    m( hasTypeHint,      4 ) \
    /* ... */

# endif  // H_GOO_STREAMING_GOO_REFLECTION_H

