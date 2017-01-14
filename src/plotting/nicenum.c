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

# include "goo_plotting/nicenum.h"

# include <stdint.h>
# include <math.h>
# include <stdlib.h>
# include <stdio.h>

struct LabelsList *
alloc_labels_list_node() {
    return malloc( sizeof(struct LabelsList) );
}

void
free_labels_list_node( struct LabelsList * nodePtr ) {
    free( nodePtr );
}

void
free_labels_list( struct LabelsList * lst ) {
    struct LabelsList * nextOne, * c;
    for( c = lst; c; c = nextOne ) {
        nextOne = c->nextPtr;
        free_labels_list_node( c );
    }
}

void
insert_labels_list_node( struct LabelsList * A, struct LabelsList * B ) {
    struct LabelsList * tail = A->nextPtr;
    A->nextPtr = B;
    B->nextPtr = tail;
}

double
nicenum( double x, uint8_t doRound ) {
    int32_t exp_;   /* exponent x */
    double f,       /* fractional part of x */
           nf;      /* nice, rounded fraction */
    exp_ = floor( log10(x) );
    f = x/pow(10, exp_);  /* between 1 and 10 */
    if( doRound ) {
        if( f < 1.5 ) { nf = 1; } else
        if( f < 3   ) { nf = 2; } else
        if( f < 7   ) { nf = 5; } else
                      { nf = 10;}
    } else {
        if( f <= 1 ) {  nf = 1; } else
        if( f <= 2 ) {  nf = 2; } else
        if( f <= 5 ) {  nf = 5; } else
                     {  nf = 10;}
    }
    return nf*pow(10., exp_);
}

uint16_t
loose_label( double * rangeMin, double * rangeMax,
             uint16_t * nTicks,
             struct LabelsList ** lstHead ) {
    int nfrac;
    uint16_t nLabels = 0;
    double d,
           graphmin, graphmax,
           range, x
           ;
    struct LabelsList * newNode = NULL;
    range = nicenum( *rangeMax - *rangeMin, 0 );
    d = nicenum( range/(*nTicks-1), 1 );
    graphmin = floor( *rangeMin/d )*d;
    graphmax =  ceil( *rangeMax/d )*d;
    nfrac = -floor( log10(d) );   // number of fractional digits to show
    if( nfrac < 0 ) {
        nfrac = 0;
    }
    char fmtStr[32] = " %%.%df",
         fmtStrOut[32];
    snprintf( fmtStrOut, sizeof(fmtStrOut), fmtStr, nfrac );
    for( x = graphmin; x < graphmax + .5*d; x += d, nLabels++ ) {
        /* Put tick mark at x with a numerical label showing
         * nfrac fraction digits */
        if( !!newNode ) {
            /* If this is not a first node */
            newNode->nextPtr = alloc_labels_list_node();
            newNode = newNode->nextPtr;
        } else {
            /* If this is a first node */
            *lstHead = newNode = alloc_labels_list_node();
        }
        newNode->position = x;
        snprintf( newNode->labelStr, sizeof(newNode->labelStr), fmtStrOut, x );
        newNode->nextPtr = NULL;
    }
    *rangeMin = graphmin;
    *rangeMax = graphmax;
    *nTicks = nLabels;
    return nfrac;
}

# ifdef STANDALONE_BUILD
int
main(int argc, char * argv[] ) {
    double min = -0.175,
           max = -0.1748;
    uint16_t nTicks = 8, nFracDgts;

    struct LabelsList * nextOne, * c;
    {
        printf( "For range [%e, %e] with %u desired number of ticks: ",
                min, max, nTicks );

        struct LabelsList * lst;
        nFracDgts = loose_label( &min, &max, &nTicks, &lst );
        
        printf( "Proposed range [%e, %e] with %u ticks (with %u digits): ",
                min, max, nTicks, nFracDgts );
        for( c = lst; c; c = nextOne ) {
            printf( " %s,", c->labelStr );
            nextOne = c->nextPtr;
        }
        printf("\b.\n");

        free_labels_list( lst );
    }
    return EXIT_SUCCESS;
}
# endif  // STANDALONE_BUILD

