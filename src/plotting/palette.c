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

# ifndef MAX
#   define MAX(a, b) (a > b ? a : b)
# endif

# define expt(a, n) pow(a, (double)(n))

/* /// palettes /// */
static double p00( double a ) { return 0; }
static double p01( double a ) { return 0.5; }
static double p02( double a ) { return 1.; }
static double p03( double a ) { return a; }
static double p04( double a ) { return a*a; }
static double p05( double a ) { return a*a*a; }
static double p06( double a ) { return pow(a, 4); }
static double p07( double a ) { return sqrt(a); }
static double p08( double a ) { return sqrt(sqrt(a)); }
static double p09( double a ) { return sin(M_PI*a/2); }
static double p10( double a ) { return cos(M_PI*a/2); }
static double p11( double a ) { return fabs(a-0.5); }
static double p12( double a ) { return pow( 2*a-1, 2 ); }
static double p13( double a ) { return sin( M_PI*a ); }
static double p14( double a ) { return fabs(cos( M_PI*a )); }
static double p15( double a ) { return sin( 2*M_PI*a ); }
static double p16( double a ) { return cos( 2*M_PI*a ); }
static double p17( double a ) { return fabs(sin( 2*M_PI*a )); }
static double p18( double a ) { return fabs(cos( 2*M_PI*a )); }
static double p19( double a ) { return fabs(sin( 4*M_PI*a )); }
static double p20( double a ) { return fabs(cos( 4*M_PI*a )); }
static double p21( double a ) { return 3*a; }
static double p22( double a ) { return 3*a-1; }
static double p23( double a ) { return 3*a-2; }
static double p24( double a ) { return fabs(3*a-1); }
static double p25( double a ) { return fabs(3*a-2); }
static double p26( double a ) { return (3*a-1)/2; }
static double p27( double a ) { return (3*a-2)/2; }
static double p28( double a ) { return fabs(3*a-1)/2; }
static double p29( double a ) { return fabs(3*a-2)/2; }
static double p30( double a ) { return a/0.32-0.78125; }
static double p31( double a ) { return 2*a-0.84; }
static double p32( double a ) {
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
static double p33( double a ) { return fabs( 2*a-0.5 ); }
static double p34( double a ) { return 2*a; }
static double p35( double a ) { return 2*a-0.5; }
static double p36( double a ) { return 2*a-1; }

ColorFunction plottingPalettes[] = {
    p00, p01, p02, p03, p04, p05, p06, p07,
    p08, p09, p10, p11, p12, p13, p14, p15,
    p16, p17, p18, p19, p20, p21, p22, p23,
    p24, p25, p26, p27, p28, p29, p30, p31,
    p32, p33, p34, p35, p36
};

goo_RGB
real_to_rgb( double val,
             double min,
             double max,
             goo_Palette * pt ) {
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
    }
    goo_RGB res = { {
            UCHAR_MAX*clcd[0],
            UCHAR_MAX*clcd[1],
            UCHAR_MAX*clcd[2],
        } };
    return res;
}

void
rgb_to_hsv(goo_RGB rgb, goo_HSV * outHsv) {
    float r = rgb.byte[0] / ((float)UCHAR_MAX),
          g = rgb.byte[1] / ((float)UCHAR_MAX),
          b = rgb.byte[2] / ((float)UCHAR_MAX)
          ;
    float max = fmaxf(fmaxf(r, g), b),
          min = fminf(fminf(r, g), b),
          delta = max - min
          ;
    if( !!delta ) {
        float hue;
        if (r == max) {
            hue = (g - b) / delta;
        } else {
            if (g == max) {
                hue = 2 + (b - r) / delta;
            } else {
                hue = 4 + (r - g) / delta;
            }
        }
        hue *= 60;
        if (hue < 0) {
            hue += 360;
        }
        outHsv->byComponent.h = hue;
    } else {
        outHsv->byComponent.h = 0;
    }
    outHsv->byComponent.s = max == 0 ? 0 : (max - min) / max;
    outHsv->byComponent.v = max;
}

