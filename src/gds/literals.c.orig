# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <limits.h>
# include "gds/literals.h"
# include "gds/errtable.h"

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
gds_interpret_escseq(const char * s) {
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

static int /* typemod to 0x80 to parse */
interpret_bin_integral(
        struct ArithmeticConstant * A,
        const char * s,
        uint8_t typemod ){
    if( 0x80 == typemod ) {
        typemod = integral_parse_typemod_postfix( s );
    }
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
                A->value.ULLongVal = *((unsigned long long *) &val);
                A->type = ULLong_T_code;
                /*printf("XXX: \"%s\" bin int, ull, %llu.\n", s, val);*/
            } else {
                A->value.LLongVal = *((long long *) &val);
                A->type = LLong_T_code;
                /*printf("XXX: \"%s\" bin int, ll, %lld.\n", s, val);*/
            }
            # else
            return -1;
            # endif
        } else {
            if( typemod & 0x1 ) {
                A->value.ULongVal = *((unsigned long *) &val);
                A->type = ULong_T_code;
                /*printf("XXX: \"%s\" bin int, ul, %lu.\n", s, val);*/
            } else {
                A->value.SLongVal = *((long *) &val);
                A->type = SLong_T_code;
                /*printf("XXX: \"%s\" bin int, l, %ld.\n", s, val);*/
            }
        }
    } else {
        if( typemod & 0x1 ) {
            A->value.UIntVal = *((unsigned *) &val);
            A->type = UInt_T_code;
            /*printf("XXX: \"%s\" bin int, u, %u.\n", s, (unsigned int) val);*/
        } else {
            A->value.SIntVal = *((int *) &val);
            A->type = SInt_T_code;
            /*printf("XXX: \"%s\" bin int, %d.\n", s, (int) val);*/
        }
    }
    return 0;
}

int
gds_interpr_integral_literal(
        struct ArithmeticConstant * A,
        const char * s,
        const uint8_t base
        ) {
    char * endCPtr = NULL;
    errno = 0;
    uint8_t typemod = integral_parse_typemod_postfix( s );

    # define check_ranges( val, min, max )                      \
    if (    (errno == ERANGE && (val == max || val == min ))    \
         || (errno != 0 && val == 0) ) {                        \
       return -2; }

    if( 2 == base ) { /* base 2 is special (can't be treated with strtoi() f-ns ) */
        interpret_bin_integral( A, s, typemod );
    } else if(
                8 != base
            && 10 != base
            && 16 != base ) {
        return -1;  /* base unsupported */
    } else {
        /* TODO:
         *  1) Check truncation errors here (strtoi has special behaviour);
         *  2) Check extra symbols on tail and warn, if they're invalid.
         */
        if( typemod & 0x2 ) {
            if( typemod & 0x4 ) {
                # ifdef EXTENDED_TYPES
                if( typemod & 0x1 ) {
                    A->value.ULLongVal = strtoull( s, &endCPtr, base );
                    A->type = ULLong_T_code;
                    check_ranges( /* TODO */ );
                } else {
                    A->value.LLongVal = strtoll( s, &endCPtr, base );
                    A->type = LLong_T_code;
                    check_ranges( /* TODO */ );
                }
                # else
                return -1;
                # endif
            } else {
                if( typemod & 0x1 ) {
                    A->value.ULongVal = strtoul( s, &endCPtr, base );
                    A->type = ULong_T_code;
                    check_ranges( A->value.ULongVal, 0, ULONG_MAX );
                } else {
                    A->value.SLongVal = strtol( s, &endCPtr, base );
                    A->type = SLong_T_code;
                    check_ranges( A->value.SLongVal, LONG_MIN, LONG_MAX );
                }
            }
        } else {
            if( typemod & 0x1 ) {
                A->value.UIntVal = strtoul( s, &endCPtr, base );
                A->type = UInt_T_code;
                check_ranges( A->value.UIntVal, 0, UINT_MAX );
            } else {
                A->value.SIntVal = strtol( s, &endCPtr, base );
                A->type = SInt_T_code;
                check_ranges( A->value.SIntVal, INT_MIN, INT_MAX );
            }
        }
        if( endCPtr == s ) {
            return -3; /* no digits were found */
        }
        if( '\0' != *endCPtr ) {
            return 1;  /* extra symbols on tail */
        }
    }
    # undef check_ranges
    return 0;
} gds_declare_result_table_for_function(gds_interpr_integral_literal) {
        { -1, "128-bytes long types are unsupported." },
        { -2, "Integer value is too large and can not be correctly interpreted." },
        { -3, "No digits where found for integral constant." },
        {  1, "Extra symbols on tail." },
    };


/* float */

int  /* TODO: use base ? */
gds_interpr_float_literal(
        struct ArithmeticConstant * A,
        const char * s,
        uint8_t base ) {
    uint8_t typemod = integral_parse_typemod_postfix( s );
    char * endptr;

    if( typemod & 0x2 ) {
        A->value.Float8Val = strtold( s, &endptr );
        A->type = Float8_T_code;
        /*printf("XXX: treat \"%s\" as double constant in decimal form: %e %a.\n",
            s, v->data.Float64Val, v->data.Float64Val);*/
    } else {
        A->value.Float4Val = strtod( s, &endptr );
        A->type = Float4_T_code;
        /*printf("XXX: treat \"%s\" as float constant in decimal form: %e %a.\n",
            s, v->data.Float32Val, v->data.Float32Val);*/
    }
    return 0;
} gds_declare_result_table_for_function(gds_interpr_float_literal) {
        { 0, "ok" },  /* unused */
        /* ... */
    };

int
gds_interpr_esc_integral(
        struct ArithmeticConstant * A,
        const char * s ){
    A->value.UByteVal = gds_interpret_escseq( s + 2 );
    A->type = UByte_T_code;
    return 0;
} gds_declare_result_table_for_function(gds_interpr_esc_integral) {
        { 0, "ok" },  /* unused */
        /* ... */
    };

gds_declare_expl_registry { uint8_t nFunction;
    gds_register_error_explainations_for( gds_interpr_integral_literal );
    gds_register_error_explainations_for( gds_interpr_esc_integral );
    gds_register_error_explainations_for( gds_interpr_float_literal );
}

# endif  /* ENABLE_GDS */

