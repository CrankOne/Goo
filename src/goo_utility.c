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

# include <unistd.h>
# include <sys/wait.h>

# include <errno.h>

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

int
sysexec_lst( const char * utilName,
             struct SysExecStatus * execStat,
             struct SysExecArgument * argsLst,
             struct SysExecEnvVar * envVarsLst,
             char sync ) {
    char ** argv_, ** itArgv_,
         ** envp_, ** itEnvp_
         ;
    int argc_ = 0,
        nenvp_ = 0
        ;
    uint16_t strL;
    struct SysExecArgument * itExecArg;
    struct SysExecEnvVar * itExecEnvVar;

    /* Do fork now. */
    pid_t waitResult, forkResult = fork();

    assert( utilName && strlen(utilName) );

    if( forkResult ) {
        /* Execution now in parent process */
        if( !sync ) {
            if( execStat ) {
                execStat->status = forkResult;
            }
            /* if not sync, return 0 immediately */
            return 1;
        } else {
            assert( execStat ); /* sync != 0, execStat must be specified! */
            /* if sync, then wait for child process to finish */
            waitResult = waitpid(forkResult, &(execStat->status), execStat->execOpts );
            if( forkResult == waitResult ) {
                if( WIFEXITED( execStat->status ) ) {
                    /* All ok. */
                    return 0;
                } else if( WIFSIGNALED( execStat->status ) ) {
                    /* Got signal */
                    return -1;
                } else {
                    /* How the child was terminated? */
                    fprintf( stderr, "sysexec_lst() error: Child process was terminated abnormally.\n" );
                    return -2;
                }
            } else {
                fprintf( stderr, "sysexec_lst() error: waitpid(%d, &(->%d), 0) -> %d\n",
                         forkResult, execStat->status, waitResult );
                return -1;
            }
        }
    } else {
        /* Execution now in child process */
        {   /* 1. Copy argv[] */
            for( itExecArg = argsLst;
                 itExecArg && itExecArg->nextArg;
                 itExecArg = itExecArg->nextArg ) {
                ++argc_;
            }
            itArgv_ = argv_ = malloc( (argc_ + 2)*sizeof(char *) );
            *itArgv_ = strdup( utilName );
            ++itArgv_;
            for( itExecArg = argsLst;
                 itExecArg && itExecArg->nextArg;
                 itExecArg = itExecArg->nextArg, itArgv_++ ) {
                *itArgv_ = strdup( itExecArg->argName );
            }
            *itArgv_ = NULL;
        }
        {   /* 2. Copy envvar */
            for( itExecEnvVar = envVarsLst;
                 itExecEnvVar && itExecEnvVar->nextVar;
                 itExecEnvVar = itExecEnvVar->nextVar ) {
                ++nenvp_;
            }
            itEnvp_ = envp_ = malloc( (nenvp_ + 1)*sizeof(char *) );
            for( itExecEnvVar = envVarsLst;
                 itExecEnvVar && itExecEnvVar->nextVar;
                 itExecEnvVar = itExecEnvVar->nextVar, ++itEnvp_ ) {
                *itEnvp_ = malloc( ( strL = ( strlen(itExecEnvVar->envVarName)
                                     + 2
                                     + strlen(itExecEnvVar->envVarValue) ) ) );
                snprintf( *itEnvp_, strL, "%s=%s", itExecEnvVar->envVarName, itExecEnvVar->envVarValue );
                //*itEnvp_ = strdup( itExecArg->argName );
            }
            *itEnvp_ = NULL;
        }
        // XXX / TODO: execvp
        # if 0
        {
            if( nenvp_ ) {
                printf( "execvpe() will be invoked with arguments:\n" );
                printf( "Envvars:\n" );
                for( c = envp_; *c; ++c ) {
                    printf( "\t%s\n", *c );
                }
            } else {
                printf( "execvp() will be invoked with arguments:\n" );
            }
            printf( "argv[]:\n" );
            for( c = argv_; *c; ++c ) {
                printf( "\t%s\n", *c );
            }
            return 0;
        }
        # endif
        # if 1
        int rc = execvpe( utilName, argv_, envp_ );
        if( -1 == rc ) {
            fprintf( stderr, "Got -1 from execvpe(). strerror()=\"%s\".\n",
                     strerror(errno) );
        } else {
            /* execvpe() returns only if an error occured. */
            fprintf( stderr, "Evaluation thread came to an unexpected place! "
                    "execvpe() returned %d.\n", rc );
        }
        exit( EXIT_FAILURE );
        # endif
    }
}


