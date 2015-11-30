# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "gds/interpreter.h"

# ifdef ENABLE_GDS

# define for_each_escseq_shortcut(m)        \
    m( 'a',     0x07 ) m( 'b',     0x08 )   \
    m( 'f',     0x0c ) m( 'n',     0x0a )   \
    m( 'r',     0x0d ) m( 't',     0x09 )   \
    m( 'v',     0x0b ) m( '\\',    0x5c )   \
    m( '\'',    0x27 ) m( '"',     0x22 )   \
    m( '?',     0x3f ) \
    /* ... */

uint32_t
interpret_escseq(const char * s) {
    uint32_t res = 0;
    switch(*s) {
        # define case_(chr, code) \
            case chr : { res = code; } break;
        for_each_escseq_shortcut(case_)
        # undef for_each_escseq_shortcut
        case 'x' : {
            char * endCPtr;
            res = strtoul( s+1, &endCPtr, 16 );
        } break;
        default : {
            char * endCPtr;
            res = strtoul( s, &endCPtr, 8 );
        }
    };
    return res;
}

/*
 * Constant values
 */

/* integral */

/* Flags:
 *  u    - 0x1
 *  l1   - 0x2
 *  l2   - 0x4
 *  s1   - 0x8  (TODO)
 *  s2   - 0x10 (TODO)
 * so:
 *  ull  - 7
 *  ul   - 3
 *  u    - 1
 *  ll   - 6
 *  l    - 2
 */
static uint8_t
integral_parse_typemod_postfix( const char * intStr ) {
    uint8_t flags = 0;
    for( const char * c = intStr;
                    *c != '\0'; ++c ) {
        if( 'u' == *c || 'U' == *c ) {
            flags |= 0x1;
        }
        if( 'l' == *c || 'L' == *c ) {
            if( flags & 0x2 ) {
                flags |= 0x4;
            } else {
                flags |= 0x2;
            }
        }
    }
    return flags;
}

struct gds_Literal *
interpret_bin_integral(
        struct gds_Parser * P,
        const char * s ){
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    unsigned long
    # ifdef EXTENDED_TYPES
        long
    # endif
        val = 0,
        mask = 0x1;
    /* parse binary notation */ {
        const char * c = s + strlen(s);
        while(*c != '1' && *c != '0') { c--; }
        while( '\0' != *c ) {
            if( '1' == *c ) {
                val |= mask;
            } else if( '0' == *c ) {
                /* do nothing */
            } else {
                break;
            }
            c--;
            mask <<= 1;
        }
    }
    /* depending on typemod, initialize appropriate field */
    if( typemod & 0x2 ) {
        if( typemod & 0x4 ) {
            # ifdef EXTENDED_TYPES
            if( typemod & 0x1 ) {
                v->data.UInt64Val = *((unsigned long long *) &val);
                v->typecode = UInt128_CT;
                /*printf("XXX: \"%s\" bin int, ull, %llu.\n", s, val);*/
            } else {
                v->data.Int64Val = *((long long *) &val);
                v->typecode = UInt128_CT;
                /*printf("XXX: \"%s\" bin int, ll, %lld.\n", s, val);*/
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error( P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt32Val = *((unsigned long *) &val);
                v->typecode = UInt64_CT;
                /*printf("XXX: \"%s\" bin int, ul, %lu.\n", s, val);*/
            } else {
                v->data.Int32Val = *((long *) &val);
                v->typecode = UInt64_CT;
                /*printf("XXX: \"%s\" bin int, l, %ld.\n", s, val);*/
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = *((unsigned *) &val);
            v->typecode = UInt32_CT;
            /*printf("XXX: \"%s\" bin int, u, %u.\n", s, (unsigned int) val);*/
        } else {
            v->data.Int32Val = *((int *) &val);
            v->typecode = UInt32_CT;
            /*printf("XXX: \"%s\" bin int, %d.\n", s, (int) val);*/
        }
    }
    return v;
}

struct gds_Literal *
interpret_oct_integral(
        struct gds_Parser * P,
        const char * s ){
    /* TODO:
     *  1) Check truncation errors here (strtoi has special behaviour);
     *  2) Check extra symbols on tail and warn, if they're invalid.
     */
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endCPtr;
    if( typemod & 0x2 ) {
        if( typemod & 0x4 ) {
            # ifdef EXTENDED_TYPES
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoull( s, &endCPtr, 8 );
                v->typecode = UInt128_CT;
            } else {
                v->data.Int64Val = strtoll( s, &endCPtr, 8 );
                v->typecode = UInt128_CT;
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error( P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoul( s, &endCPtr, 8 );
                v->typecode = UInt64_CT;
            } else {
                v->data.Int64Val = strtol( s, &endCPtr, 8 );
                v->typecode = UInt64_CT;
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = strtoul( s, &endCPtr, 8 );
            v->typecode = UInt32_CT;
        } else {
            v->data.Int32Val = strtol( s, &endCPtr, 8 );
            v->typecode = Int32_CT;
        }
    }
    return v;
}

struct gds_Literal *
interpret_hex_integral(
        struct gds_Parser * P,
        const char * s ){
    /* TODO:
     *  1) Check truncation errors here (strtoi has special behaviour);
     *  2) Check extra symbols on tail and warn, if they're invalid.
     */
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endCPtr;
    if( typemod & 0x2 ) {
        if( typemod & 0x4 ) {
            # ifdef EXTENDED_TYPES
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoull( s, &endCPtr, 16 );
                v->typecode = UInt128_CT;
            } else {
                v->data.Int64Val = strtoll( s, &endCPtr, 16 );
                v->typecode = UInt128_CT;
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error( P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoul( s, &endCPtr, 16 );
                v->typecode = UInt64_CT;
            } else {
                v->data.Int64Val = strtol( s, &endCPtr, 16 );
                v->typecode = UInt64_CT;
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = strtoul( s, &endCPtr, 16 );
            v->typecode = UInt32_CT;
        } else {
            v->data.Int32Val = strtol( s, &endCPtr, 16 );
            v->typecode = Int32_CT;
        }
    }
    return v;
}

struct gds_Literal *
interpret_esc_integral(
        struct gds_Parser * P,
        const char * s ){
    struct gds_Literal * v = gds_parser_new_Literal(P);
    v->data.UInt8Val = interpret_escseq( s + 2 );
    return v;
}

struct gds_Literal *
interpret_dec_integral(
        struct gds_Parser * P,
        const char * s ){
    /* TODO:
     *  1) Check truncation errors here (strtoi has special behaviour);
     *  2) Check extra symbols on tail and warn, if they're invalid.
     */
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endCPtr;
    if( typemod & 0x2 ) {
        if( typemod & 0x4 ) {
            # ifdef EXTENDED_TYPES
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoull( s, &endCPtr, 10 );
                v->typecode = UInt128_CT;
            } else {
                v->data.Int64Val = strtoll( s, &endCPtr, 10 );
                v->typecode = UInt128_CT;
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error( P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt64Val = strtoul( s, &endCPtr, 10 );
                v->typecode = UInt64_CT;
            } else {
                v->data.Int64Val = strtol( s, &endCPtr, 10 );
                v->typecode = Int64_CT;
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = strtoul( s, &endCPtr, 10 );
            v->typecode = UInt32_CT;
        } else {
            v->data.Int32Val = strtol( s, &endCPtr, 10 );
            v->typecode = Int32_CT;
        }
    }
    return v;
}

/* float */

struct gds_Literal *
interpret_float_dec(
        struct gds_Parser * P,
        const char * s ){
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endptr;

    if( typemod & 0x2 ) {
        v->data.Float64Val = strtold( s, &endptr );
        v->typecode = Float64_CT;
        /*printf("XXX: treat \"%s\" as double constant in decimal form: %e %a.\n",
            s, v->data.Float64Val, v->data.Float64Val);*/
    } else {
        v->data.Float32Val = strtod( s, &endptr );
        v->typecode = Float32_CT;
        /*printf("XXX: treat \"%s\" as float constant in decimal form: %e %a.\n",
            s, v->data.Float32Val, v->data.Float32Val);*/
    }
    return v;
}

struct gds_Literal *
interpret_float_hex(
        struct gds_Parser * P,
        const char * s ){
    struct gds_Literal * v = gds_parser_new_Literal(P);
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endptr;

    if( typemod & 0x2 ) {
        v->data.Float64Val = strtold( s, &endptr );
        v->typecode = Float64_CT;
        /*printf("XXX: treat \"%s\" as double constant in hexidecimal form: %e %a.\n",
            s, v->data.Float64Val, v->data.Float64Val);*/
    } else {
        v->data.Float32Val = strtod( s, &endptr );
        v->typecode = Float32_CT;
        /*printf("XXX: treat \"%s\" as float constant in hexidecimal form: %e %a.\n",
            s, v->data.Float32Val, v->data.Float32Val);*/
    }
    return v;
}

/* string literal */

struct gds_Literal *
interpret_string_literal(
        struct gds_Parser * P,
        const char * s ){
    size_t len = strlen(s);
    char * copy = gds_heap_alloc(P->literals, len+1);
    strcpy( copy, s );
    struct gds_Literal * v = gds_parser_new_Literal(P);
    v->data.stringValPtr = copy;
    return v;
}

struct gds_Literal *
interpret_logic_true( struct gds_Parser * P ) {
    struct gds_Literal * v = gds_parser_new_Literal(P);
    v->typecode = logic_CT;
    v->data.logicVal = 0xff;
    return v;
}

struct gds_Literal *
interpret_logic_false( struct gds_Parser * P ) {
    struct gds_Literal * v = gds_parser_new_Literal(P);
    v->typecode = logic_CT;
    v->data.logicVal = 0x0;
    return v;
}

struct gds_Literal *
gds_literal_heapcopy( struct gds_Parser * P,  struct gds_Literal * o ) {
    struct gds_Literal * c = gds_heap_alloc(P->literals, sizeof(struct gds_Literal));
    memcpy(c, o, sizeof(struct gds_Literal));
    return c;
}

void
gds_literal_heapfree( struct gds_Parser * P, struct gds_Literal * inst ) {
    if( !inst ) { return; }
    gds_heap_erase(P->literals, inst);
}

# endif  /* ENABLE_GDS */

