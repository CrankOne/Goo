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

# include <math.h>
# include <stdio.h>
# include <time.h>
# include <string.h>
# include <stdlib.h>
# include <assert.h>

# include "goo_utility.h"

double
goo_dstr_triangular( const double c ) {
    double u = rand()/(double)(RAND_MAX);
    return u < c ? sqrt(c*u) : 1 - sqrt( (1-c)*(1-u) );
}

/*
 * Time
 */

static char timebf[32];

const char *
hctime() {
    clock_t cTime_ = clock();
    double cTime = (10*cTime_/((double) CLOCKS_PER_SEC));
    if(cTime < 0) {
        return "time_unsupp";
    }
    snprintf(timebf, 8, "%.2f", cTime );
    return timebf;
}

const char *
get_timestamp() {
    char sbf[8];
    char tbf[32];
    strncpy( sbf, hctime(), 8 );
    time_t rawtime;
    struct tm *info;
    time( &rawtime );
    info = localtime( &rawtime );
    strftime(tbf, 32, "%y/%m/%d/%H/%M/%S", info );
    sprintf(timebf, "%s.%s", tbf, sbf );
    return timebf;
}

unsigned long __ulongFactorialTable[] = {
    1,    1,     2,      6,       24,       120,       720,
    5040, 40320, 362880, 3628800, 39916800, 479001600
};

unsigned long
factorial_16( unsigned char a ) {
    assert( a < 13 );
    return __ulongFactorialTable[a];
}

/*
 * Memsize formatters
 */

static struct DictEntry {
    const char nm[3];
    unsigned long val;
} sizesDict[] = {
    /* > Pb is unsupported */
    {"Tb",   1024*1024*1024*1024UL,     },
    {"Gb",   1024*1024*1024,            },
    {"Mb",   1024*1024,                 },
    {"kb",   1024,                      },
    {"b",    1,                         },
    {{'\0'}, 0                          },
};

static const struct DictEntry *
nearest_entry( unsigned long a ) {
    const struct DictEntry * c;
    for( c = sizesDict; '\0' != c->nm[0]; ++c ) {
        if( a/c->val ) break;
    }
    if( '\0' == c->nm[0] ) c = sizesDict;
    return c;
}

char *
rounded_mem_size( unsigned long toPrint,
                  char * buffer,
                  unsigned char bufLength ) {
    const struct DictEntry * c;

    if( !toPrint ){
        snprintf( buffer, bufLength, "0b" );
        return buffer;
    }

    c = nearest_entry( toPrint );

    snprintf( buffer, bufLength, "%lu%s", (unsigned long) round(((double) toPrint)/((double) c->val)), c->nm );

    return buffer;
}

struct SlicedEntry {
    char txtBuf[8];
    unsigned short n;
};

char *
fancy_mem_size( unsigned long toPrint,
               char * buffer,
               unsigned char bufLength ) {
    if( !toPrint ) {
        snprintf( buffer, bufLength, "0b" );
        return buffer;
    }
    struct SlicedEntry se[5] = {
            { "", 0 },  { "", 0 },  { "", 0 },
            { "", 0 },  { "", 0 }
        };
    unsigned char i, got = 0;
    unsigned long nearest = sizesDict[0].val;

    unsigned long remainder = toPrint;
    for( i = 0; i < 5; ++i ) {
        se[i].n = remainder/nearest;
        if(se[i].n || got){
            got = 1;
            snprintf( se[i].txtBuf, 8, "%u%c", se[i].n, (i == 4 ? 'b' : '`') );
            remainder -= se[i].n*nearest;
        }
        nearest /= 1024;
    }
    snprintf( buffer, bufLength, "%s%s%s%s%s", se[0].txtBuf, se[1].txtBuf,
                                               se[2].txtBuf, se[3].txtBuf,
                                               se[4].txtBuf);
    return buffer;
}

static char __rndMemSizeBf[128];

char *
rounded_mem_size_stb( unsigned long toPrint ) {
    return rounded_mem_size( toPrint, __rndMemSizeBf,
                             sizeof(__rndMemSizeBf) );
}

static char __fancyMemSizeBf[128];

char *
fancy_mem_size_stb( unsigned long toPrint ) {
    return fancy_mem_size( toPrint, __fancyMemSizeBf,
                           sizeof(__fancyMemSizeBf) );
}

/** Utility function providing string substitution. All the strings
 * provided to the function are required to do not overlap. Only
 * rudimentary check for non-NULL, non-emptiness are provided
 * inside this function.
 *
 * \param sourceString is a pointer to the string containing sequence(s)
 * that is (are) to be substituted
 * \param origToken is a sequence to be substituted
 * \param replacement is a sequence to be substituted instead of `origToken`.
 * \param buffer a destination buffer, where output is to be written.
 * \param bufferLength is a length of target buffer.
 *
 * \returns -1 if at least one of arguments provided is NULL or
 * has zero length;
 * \returns -2 if provided buffer length is insufficient.
 * \returns positive number corresponding to number of provided
 * substitutions.
 */
int
replace_string( const char * restrict sourceString,
                const char * restrict origToken,
                const char * restrict replacement,
                char * restrict buffer,
                const size_t bufferLength) {
    if(     !sourceString
         || !buffer
         || !origToken
         || !bufferLength
         || !replacement ) return -1;  /* bad argument */
    /* routine parameters */
    # define MAX_OCCURENCES 32
    /* variables */
    const size_t replacementLength = strlen(replacement),
                        origLength = strlen(origToken),
                      sourceLength = strlen(sourceString)
                                   ;
    size_t nOccurences = 0;
    const char * occurences[MAX_OCCURENCES],
               * occurence = sourceString,
               * cSrc, * replPtr;
    char * cDst;
    if(     !replacementLength
         || !origLength
         || !sourceString ) {
        return -1;
    }
    /* Find all matches and cache their positions. */
	while(     (occurence = strstr(occurence, origToken)) != NULL
            && nOccurences < MAX_OCCURENCES ) {
        occurences[nOccurences++] = occurence;
        ++occurence;
    }
    occurences[nOccurences] = 0;
    if( bufferLength < sourceLength - nOccurences*( replacementLength - origLength ) ) {
        return -2;
    }
    nOccurences = 0;
    for( cSrc = sourceString, cDst = buffer; '\0' != *cSrc; ++cDst, ++cSrc ) {
        if( cSrc != occurences[nOccurences] ) {
            *cDst = *cSrc;
            continue;
        }
        /* occurence */
        cSrc += origLength - 1;
        for( replPtr = replacement; '\0' != *replPtr; ++replPtr ) {
            *(cDst++) = *replPtr;
        }
        nOccurences++;
        cDst--;
    }
    *cDst = '\0';
    return nOccurences;
}
