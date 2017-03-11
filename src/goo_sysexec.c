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

# include "goo_sysexec.h"

# include <unistd.h>
# include <sys/wait.h>
# include <sys/select.h>
# include <fcntl.h>
# include <errno.h>
# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <assert.h>

const uint8_t
    gooExec_noSync              = 0x1,
    gooExec_noStdOutHandle      = 0x2,
    gooExec_noStdErrHandle      = 0x4,
    gooExec_noStdInHandle       = 0x8,
    gooExec_noStdStreamsHandle  = 0x2  /*gooExec_noStdOutHandle*/
                                | 0x4  /*gooExec_noStdErrHandle*/
                                | 0x8  /*gooExec_noStdInHandle*/
                                ;

const int8_t
    gooExec_finished            =  0,
    gooExec_detached            =  1,
    gooExec_childGotSignal      = -1,
    gooExec_error               = -2
    ;

void
goo_sysexec_close_pipe_handlers( struct SysExecStatus * execStat ) {
    int n;
    
    /* NULL allowed */
    if( !execStat ) return;

    for( n = 0; n < 3; ++n ) {
        if( execStat->stdStreamDescs[n][0] ) {
            close(execStat->stdStreamDescs[n][0]);
            execStat->stdStreamDescs[n][0] = 0;
        }
        if( execStat->stdStreamDescs[n][1] ) {
            close(execStat->stdStreamDescs[n][1]);
            execStat->stdStreamDescs[n][1] = 0;
        }
    }
}

static void
_static_goo_exec_child_noreturn( const char *,
                                 struct SysExecStatus *,
                                 const struct SysExecArgument *,
                                 const struct SysExecEnvVar *,
                                 uint8_t flags ) __attribute__ ((noreturn));
static void
_static_goo_exec_child_noreturn( const char * utilName,
                                 struct SysExecStatus * execStat,
                                 const struct SysExecArgument * argsLst,
                                 const struct SysExecEnvVar * envVarsLst,
                                 uint8_t flags ) {
    char ** argv_, ** itArgv_,
         ** envp_, ** itEnvp_
         ;
    int argc_ = 0,
        nenvp_ = 0,
        execResult
        ;
    uint16_t strL;
    const struct SysExecArgument * itExecArg;
    const struct SysExecEnvVar * itExecEnvVar;

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

    /* Pipelining. */
    if( execStat->stdStreamDescs[0][0] ) {
        while( (dup2(execStat->stdStreamDescs[0][1], fileno(stdout)) == -1)
            && (errno == EINTR) ) {}
        close(execStat->stdStreamDescs[0][1]);
        execStat->stdStreamDescs[0][1] = 0;
        /*close(execStat->stdStreamDescs[0][0]);*/
    }
    if( execStat->stdStreamDescs[1][0] ) {
        while( (dup2(execStat->stdStreamDescs[1][1], fileno(stderr)) == -1)
            && (errno == EINTR) ) {}
        close(execStat->stdStreamDescs[1][1]);
        execStat->stdStreamDescs[1][1] = 0;
        /*close(execStat->stdStreamDescs[1][0]);*/
    }

    if( itExecEnvVar ) {
        execResult = execvpe( utilName, argv_, envp_ );
    } else {
        execResult = execvp(  utilName, argv_ );
    }

    if( -1 == execResult ) {
        perror( "execvpe() error: ");
    }
    abort();  /* << normally, should never get here. */
}

int
goo_sysexec_lst( const char * utilName,
                 struct SysExecStatus * execStat,
                 const struct SysExecArgument * argsLst,
                 const struct SysExecEnvVar * envVarsLst,
                 uint8_t flags ) {
    pid_t waitResult, forkResult;

    assert( execStat );
    assert( utilName && strlen(utilName) );

    # define open_pipe_for( streamName, idx, verbosename )          \
    if(!(flags & gooExec_no ## streamName ## Handle)) {             \
        if( -1 == pipe( execStat->stdStreamDescs[ idx ] ) ) {       \
            perror( "goo_sysexec_lst() : pipe() returned -1 for "   \
                        verbosename ". " );                         \
            bzero(  execStat->stdStreamDescs[ idx ],                \
                    2*sizeof(execStat->stdStreamDescs[0][0]) );     \
        } else {                                                    \
            if( fcntl(  execStat->stdStreamDescs[idx][0],           \
                        F_SETFD, FD_CLOEXEC) == -1 ) {              \
                perror("goo_sysexec_lst() : fcntl() for "           \
                            verbosename ". ");                      \
                exit(1);                                            \
            }                                                       \
        }                                                           \
    } else {                                                        \
        bzero( execStat->stdStreamDescs[ idx ],                     \
                    2*sizeof(execStat->stdStreamDescs[0][0]) );     \
    }
    open_pipe_for( StdOut, 0, "standard output stream" );
    open_pipe_for( StdErr, 1, "standard error stream" );
    /*TODO: open_pipe_for( StdIn,  2 )*/
    bzero( execStat->stdStreamDescs[2], 2*sizeof(execStat->stdStreamDescs[0][0]) );
    # undef open_pipe_for

    /* Do fork now. */
    if( (forkResult = fork()) < 0 ) {
        perror( "goo_sysexec_lst() was unable to fork process: " );
        goo_sysexec_close_pipe_handlers( execStat );
        return gooExec_error;
    }

    if( 0 == forkResult ) {
        /* never returns */
        _static_goo_exec_child_noreturn(
                utilName,
                execStat,
                argsLst,
                envVarsLst,
                flags );
        abort();  /* << should never get here. */
    }

    /* Execution now in parent process */
    close( execStat->stdStreamDescs[0][1] );
    close( execStat->stdStreamDescs[1][1] );
    if( flags & gooExec_noSync ) {
        if( execStat ) {
            execStat->status = forkResult;
        }
        /* if not sync, return 1 immediately */
        return gooExec_detached;
    }

    /* wait for child process to finish */
    waitResult = waitpid(forkResult, &(execStat->status), execStat->execOpts );

    if( waitResult < 0 ) {
        perror( "goo_sysexec_lst() was unable to wait for a child process: " );
        goo_sysexec_close_pipe_handlers( execStat );
        return gooExec_error;
    }

    if( WIFEXITED( execStat->status ) ) {
        return gooExec_finished;
    } else if( WIFSIGNALED( execStat->status ) ) {
        return gooExec_childGotSignal;
    }
    perror( "goo_sysexec_lst() met unknown child process termination." );
    return gooExec_error;
}

