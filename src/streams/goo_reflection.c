# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include "streams/goo_reflection.h"

# define declare_const( name, bitshift ) \
    static const UByte name = 1 << (0x7 - 0x ## bitshift); \
    static const UByte name ## _bitNo = bitshift;
    for_each_goo_reflection_serialized_flags(declare_const)
# undef declare_const

# define is_last_reflection_byte( o ) \
    (!((*o)&(useNextByte)))

Size
reflection_length( const UByte * r ) {
    if( !r ) { return 0; }
    Size length = 0;
    for( const UByte * rit = r; !is_last_reflection_byte(rit); ++rit ) {
        length++;
    }
    return length;
}

Size
copy_reflection_bytes( UByte * __restrict dest,
                       const UByte * __restrict src ) {
    Size copyLength = 0;
    UByte * rdest = dest;
    for( const UByte * rsrc = src; !is_last_reflection_byte(rsrc); ++rsrc, copyLength++ ) {
        *rdest = *rsrc;
    }
    return copyLength;
}

UByte *  /* (re-)allocates reflection and sets new bits to zero */
gain_reflection( UByte * previous, const Size newLength ) {
    UByte * nr = (UByte *) malloc(newLength);
    bzero(nr, newLength);
    if( previous ) {
        const Size copyLength = copy_reflection_bytes( nr, previous );
        nr[copyLength-1] |= useNextByte;
    }
    return nr;
}

void
textualize_reflection_info(const UByte * r, FILE * outFile) {
    char byteText[10] = "uvwx ????";

    for( UByte bitno = 0; bitno < 8; ++bitno ) {
        UByte charNo = bitno < 4 ? bitno : bitno + 1;
        if( *r & (1 << bitno) ) {
            byteText[charNo] = '1';
        } else {
            byteText[charNo] = '0';
        }
    }

    if( *r & isVariadic ) {
        byteText[isVariadic_bitNo] = 'v';
    } else {
        byteText[isVariadic_bitNo] = 'u';
        if( *r & isArithmetic ) {
            byteText[isArithmetic_bitNo] = 'a';
        }
    }

    if( *r & isArray ) {
        byteText[isArray_bitNo] = '+';
    } else {
        byteText[isArray_bitNo] = '1';
    }

    /* TODO: size data, array data, whatsoever */
    fprintf( outFile, "%p : %s\n", r, byteText );
}

