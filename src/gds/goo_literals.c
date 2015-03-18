# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include "gds/goo_interpreter.h"

# ifdef ENABLE_GDS

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
    printf("XXX: treat \"%s\" as integral constant in octal form.\n", s);
    return 0;
}

struct gds_Literal *
interpret_hex_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in hexidecimal form.\n", s);
    return 0;
}

struct gds_Literal *
interpret_esc_integral(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as integral constant in escape sequence form.\n", s);
    return 0;
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
                printf("XXX: \"%s\" dec int, uul.\n", s);
            } else {
                v->data.Int64Val = strtoll( s, &endCPtr, 10 );
                v->typecode = UInt128_CT;
                printf("XXX: \"%s\" dec int, ll.\n", s);
            }
            # else
            char bf[64];
            snprintf(bf, 64, "\"%s\" -- 128-bit integer is unsupported", s );
            gds_error( P, bf );
            # endif
        } else {
            if( typemod & 0x1 ) {
                v->data.UInt32Val = strtoul( s, &endCPtr, 10 );
                v->typecode = UInt64_CT;
                printf("XXX: \"%s\" dec int, ul.\n", s);
            } else {
                v->data.Int32Val = strtol( s, &endCPtr, 10 );
                v->typecode = UInt64_CT;
                printf("XXX: \"%s\" dec int, l.\n", s);
            }
        }
    } else {
        if( typemod & 0x1 ) {
            v->data.UInt32Val = strtoul( s, &endCPtr, 10 );
            v->typecode = UInt32_CT;
            printf("XXX: \"%s\" dec int, u.\n", s);
        } else {
            v->data.Int32Val = strtol( s, &endCPtr, 10 );
            v->typecode = UInt32_CT;
            printf("XXX: \"%s\" dec int.\n", s);
        }
    }
    return v;
}

/* float */

struct gds_Literal *
interpret_float_dec(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in decimal form.\n", s);
    return 0;
}

struct gds_Literal *
interpret_float_hex(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as float constant in hexidecimal form.\n", s);
    return 0;
}

/* string literal */

struct gds_Literal *
interpret_string_literal(
        struct gds_Parser * P,
        const char * s ){
    printf("XXX: treat \"%s\" as string literal of length %zd.\n",
            s, strlen(s) );
    return 0;
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

# endif  /* ENABLE_GDS */

