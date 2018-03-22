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

# ifndef H_GOO_SYSEXEC_H
# define H_GOO_SYSEXEC_H

# include "goo_types.h"

# ifdef __cplusplus
extern "C" {
# endif

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

struct SysExecStatus {
    /** A `waitpid()` exit status variable. See `man 3p wait` for
     * explaination. If `gooExec_noSync` flag is specified, this
     * field will contain child process PID. */
    int status; /* todo: union with pid_t? */
    /** The `options` variable of waitpid() invokation. */
    int execOpts;
    /** Stores descriptors for pipelines. Note that in parent process,
     * the second handler (pipeline input) will be always set to 0 
     * (unused) --- they are used internally by forked child.
     * @TODO: 3rd is for stdin which is currently not implemented. */
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

/**@brief A system-execution function combining functions similar to
 * popen()/system()/execl()/etc.
 *
 * By default (if if all arguments except flags variable are non-zero),
 * will fork current process and delegate execution to system process
 * via execl(). Parent process shall wait for child. Note, that `execStat`
 * structure have to be initialized.
 *
 * Upon finish with default settings, the `execStat` struct will contain:
 *  - `waitpid()` status in `.status` field (see `WEXITSTATUS` macro of
 *    `man 3p wait` to obtain process exit code).
 *  - `stdStreamDescs` contains pipeline pipeline descriptors in order:
 *    for stdout, stderr, and stdin (TODO: stdin currently not
 *    implemented).
 *
 * Behaviour can be modified by `flags` argument:
 *  - `gooExec_noSync` flag causes this function to not wait for child
 *    process to finish. When flag is on, the `status` field of
 *    execStat _will be set to child process PID_.
 *  - `gooExec_noStd[Out|Err|In]Handle` controls suppression pipeline
 *    redirection of corresponding streams. When set, the corresponding
 *    pipeline stream descriptor will be set to 0 indicating that is not
 *    used.
 *  For convinience, there is a shortcut meaning "do not redirect all
 *  streams" --- `gooExec_noStdStreamsHandle`.
 *
 * @arg utilName    --- path to an executable to be ran.
 * @arg execStat    --- a non-null pointer to a struct initially managing
 *                  the `waitpid()` call (`.execOpts`)
 *                  and, upon finish, keepeing the execution result and
 *                  pipeline descriptors.
 * @arg argsLst     --- (can be NULL) a pointer to list of command-line
 *                  arguments (i.e. `argv[]` of target process).
 * @arg envVarsLst  --- (can be NULL) A list of supplement environment
 *                  variables.
 * @arg flags       --- flags steering the execution of parent process
 *                  (sync/async, pipelining).
 * */
int goo_sysexec_lst( const char * utilName,
                     struct SysExecStatus * execStat,
                     const struct SysExecArgument * argsLst,
                     const struct SysExecEnvVar * envVarsLst,
                     uint8_t flags );

void goo_sysexec_close_pipe_handlers( struct SysExecStatus * execStat );

# ifdef __cplusplus
}  /* extern "C" */
# endif

# endif

