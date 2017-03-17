# include <goo_sysexec.h>

# include <unistd.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <errno.h>
# include <sys/wait.h>

# if 0
struct SysExecArgument args[] = {
    { "foo", NULL },
    { "bar", NULL },
    { "zum", NULL },
    { NULL,  NULL }
};

struct SysExecEnvVar envVars[] = {
    { "PATH", "some/where:nowhere", NULL },
    { "BLAH", "blah-blah:blah", NULL },
    { NULL, NULL, NULL },
};
# endif

int
invrun( struct SysExecArgument * args,
        struct SysExecEnvVar * envVars ) {
    int sysexecRC;
    struct SysExecArgument * itA;
    struct SysExecEnvVar * itV;
    struct SysExecStatus execStat;
    uint8_t execFlags = 0, i;

    # ifdef NCHECK_PIPELINED
    execFlags |= gooExec_noStdStreamsHandle;
    # endif
    execFlags |= gooExec_noSync;

    bzero(&(execStat), sizeof(struct SysExecStatus));

    for( itA = args; itA && itA->argName; ++itA ) {
        itA->nextArg = (itA+1);
    }
    for( itV = envVars; itV && itV->envVarName; ++itV ) {
        itV->nextVar = (itV+1);
    }

    if( (sysexecRC = goo_sysexec_lst(
                "../../goo/utils/SystemTests/script/2call.sh",
                &execStat,
                args,
                envVars,
                execFlags )) != /*gooExec_finished*/ gooExec_detached ) {
        fprintf( stderr, "Got %d from goo_sysexec_lst() call. Terminating.\n", sysexecRC );
        abort();
    }

    # ifndef NCHECK_PIPELINED
    char buffer[4096];
    for( i = 0; i < 2; ++i ) {
        printf( "Reading pipeline of %s stream...\n", (0 == i ? "standard output" : "standard error") );
        while (1) {
            ssize_t count = read( execStat.stdStreamDescs[i][0], buffer, sizeof(buffer));
            if (count == -1) {
                if( errno == EINTR ) {
                    continue;
                } else {
                    perror("read(): ");
                    abort();
                }
            } else if( 0 == count ) {
                break;
            } else {
                printf( "%s stream content (%zu bytes) \"\"\"\n",
                        (0 == i ? "Standard output" : "Standard error"),
                        count );
                puts( buffer );
                puts( "\"\"\"" );
                //handle_child_process_output( buffer, count );
            }
        }
        close( execStat.stdStreamDescs[i][0] );
        wait(0);
    }
    # endif

    return WEXITSTATUS( execStat.status );
}

int
main( int argc, char * argv[] ) {
    int rc;

    {
        if( 1 != (rc = invrun( NULL, NULL )) ) {
            fprintf( stderr, "Got return code %d from invokation. Expected 1.\n", rc );
            return EXIT_FAILURE;
        }
    }
    printf( "(test): case 1 --- ok\n" );
    {
        struct SysExecArgument args[] = {
            { "--arg1", NULL },
            { "one", NULL },
            { NULL, NULL }
        };
        if( 2 != (rc = invrun( args, NULL )) ) {
            fprintf( stderr, "Got return code %d from invokation. Expected 2.\n", rc );
            return EXIT_FAILURE;
        }
    }
    printf( "(test): case 2 --- ok\n" );
    return EXIT_SUCCESS;
}

