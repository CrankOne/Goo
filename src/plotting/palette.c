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

# include "goo_plotting/palette.h"
# include "goo_types.h"
# include "goo_exception.hpp"

# include <stdlib.h>
# include <stdio.h>
# include <assert.h>
# include <string.h>
# include <errno.h>
# include <math.h>
# include <pthread.h>
# include <limits.h>
# include <unistd.h>

# ifdef PNG_FOUND
# ifndef NDEBUG
# define PNG_DEBUG 3
# else
# define PNG_DEBUG 0
# endif
# include <png.h>
# endif

# ifndef MAX
#   define MAX(a, b) (a > b ? a : b)
# endif

# define expt(a, n) pow(a, (double)(n))

UByte nPlottingTicksDefault = 10;

typedef goo_Palette Palette;
typedef goo_RGB RGB;

/* /// palettes /// */
double p00( double a ) { return 0; }
double p01( double a ) { return 0.5; }
double p02( double a ) { return 1.; }
double p03( double a ) { return a; }
double p04( double a ) { return a*a; }
double p05( double a ) { return a*a*a; }
double p06( double a ) { return pow(a, 4); }
double p07( double a ) { return sqrt(a); }
double p08( double a ) { return sqrt(sqrt(a)); }
double p09( double a ) { return sin(M_PI*a/2); }
double p10( double a ) { return cos(M_PI*a/2); }
double p11( double a ) { return fabs(a-0.5); }
double p12( double a ) { return pow( 2*a-1, 2 ); }
double p13( double a ) { return sin( M_PI*a ); }
double p14( double a ) { return fabs(cos( M_PI*a )); }
double p15( double a ) { return sin( 2*M_PI*a ); }
double p16( double a ) { return cos( 2*M_PI*a ); }
double p17( double a ) { return fabs(sin( 2*M_PI*a )); }
double p18( double a ) { return fabs(cos( 2*M_PI*a )); }
double p19( double a ) { return fabs(sin( 4*M_PI*a )); }
double p20( double a ) { return fabs(cos( 4*M_PI*a )); }
double p21( double a ) { return 3*a; }
double p22( double a ) { return 3*a-1; }
double p23( double a ) { return 3*a-2; }
double p24( double a ) { return fabs(3*a-1); }
double p25( double a ) { return fabs(3*a-2); }
double p26( double a ) { return (3*a-1)/2; }
double p27( double a ) { return (3*a-2)/2; }
double p28( double a ) { return fabs(3*a-1)/2; }
double p29( double a ) { return fabs(3*a-2)/2; }
double p30( double a ) { return a/0.32-0.78125; }
double p31( double a ) { return 2*a-0.84; }
double p32( double a ) {
    if( a < 0.25 ) {
        return 4*a;
    } else if( a < 0.42 ) {
        return 1;
    } else if( a < 0.92 ) {
        return -2*a+1.84;
    } else {
        return a/0.08-11.5;
    }
}
double p33( double a ) { return fabs( 2*a-0.5 ); }
double p34( double a ) { return 2*a; }
double p35( double a ) { return 2*a-0.5; }
double p36( double a ) { return 2*a-1; }

ColorFunction plottingPalettes[] = {
    p00, p01, p02, p03, p04, p05, p06, p07,
    p08, p09, p10, p11, p12, p13, p14, p15,
    p16, p17, p18, p19, p20, p21, p22, p23,
    p24, p25, p26, p27, p28, p29, p30, p31,
    p32, p33, p34, p35, p36
};

RGB
dbl2rgb( double val,
         double min, double max,
         Palette * pt ) {
    double proto = (val-min)/(max-min);
    double clcd[3] = {
            (pt->RedF(   proto )),
            (pt->GreenF( proto )),
            (pt->BlueF(  proto )),
        };
    for( UByte i = 0 ; i < 3; ++i ) {
        if( clcd[i] > 1. ) {
            clcd[i] = 1.;
        } else if ( clcd[i] < 0 ) {
            clcd[i] = 0.;
        }
        if( pt->invert & (int) pow( 0x2, i ) ) {
            clcd[i] = 1 - clcd[i];
        }
    }
    RGB res = {
            UCHAR_MAX*clcd[0],
            UCHAR_MAX*clcd[1],
            UCHAR_MAX*clcd[2],
        };
    return res;
}

