# ifndef H_GOO_UTILITY_H
# define H_GOO_UTILITY_H

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

# include "goo_types.h"

# define STRINGIFY_MACRO_ARG(a) _STRINGIFY_MACRO_ARG(a)
# define _STRINGIFY_MACRO_ARG(a) #a

# ifdef __cplusplus
extern "C" {
# endif

/** Generates a triangle-distributed numbers with given width(?). */
double goo_dstr_triangular( const double c );

/** Returns current timestamp. Uses internal static buffer. */
const char * hctime();

/** Returns fancy text timestamp. Uses internal static buffer. */
const char * get_timestamp();

/** Quick factorial up to 12. */
unsigned long factorial_16( unsigned char );

/** Writes rounded memory size up to Tb into buffer and returns buffer. */
char *
rounded_mem_size( unsigned long toPrint,
                  char * buffer,
                  unsigned char bufLength );

/**@brief Writes fancy-formatted size string.
 *
 * Produces result in form of:
 *   `[N-Tb?]\`[N-Gb?]\`[N-Mb?]\`[N-kb?]\`[N-b]`
 * Where N-xx is a number of terrabytes/gigabytes/... that is contained
 * by provided size argument number. It is not a decimal number, just a
 * convinient way to approximately estimate actual size.
 */
char *
fancy_mem_size( unsigned long toPrint,
                  char * buffer,
                  unsigned char bufLength );

/** A static buffer (uses own) version of rounded_mem_size(). */
char * rounded_mem_size_stb( unsigned long toPrint );

/** A static buffer (uses own) version of fancy_mem_size(). */
char * fancy_mem_size_stb( unsigned long toPrint );

/** List structure for argv[] parameters of sysexec_lst() */
struct SysExecArgument {
    char * argName;
    struct SysExecArgument * nextArg;
};

/** List structure for environment variable of sysexec_lst() */
struct SysExecEnvVar {
    char * envVarName,
         * envVarValue
         ;
    struct SysExecEnvVar * nextVar;
};

/* TODO: IO handlers! */
struct SysExecStatus {
    /** A waitpid() exit status variable. See man 3p wait for
     * explaination. */
    int status;
    /** The `options` variable of waitpid() invokation. */
    int execOpts;
    /** Stores descriptors for pipelines. */
    int stdStreamDescs[3][2];
};

extern const uint8_t
    gooExec_noSync,
    gooExec_noStdOutHandle,
    gooExec_noStdErrHandle,
    gooExec_noStdInHandle,
    gooExec_noStdStreamsHandle
    ;

extern const int8_t
    gooExec_finished,
    gooExec_detached,
    gooExec_childGotSignal,
    gooExec_error
    ;

/** Safe wrapper for system() call. Uses fork()/execl() in combination.
 * Can wait for child process to stop, if sync != 0. If no additional
 * environment variables should be specified, NULL ptr is acceptable for
 * fourth argument.
 *
 * TODO: further doc
 * */
int goo_sysexec_lst( const char * utilName,
                     struct SysExecStatus * execStat,
                     const struct SysExecArgument * argsLst,
                     const struct SysExecEnvVar * envVarsLst,
                     uint8_t flags );

void goo_sysexec_close_pipe_handlers( struct SysExecStatus * execStat );

# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_UTILITY_H */

