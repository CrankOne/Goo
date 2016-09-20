# include <goo_utility.h>
# include <stdlib.h>
# include <stdio.h>
# include <string.h>

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
    uint8_t execFlags = 0;
    # if 1
    /*  ^^^ Disable this to pipeline child process standard streams
     * into anonymous parent's. */
    execFlags |= gooExec_noStdStreamsHandle;
    # endif

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
                execFlags )) != gooExec_finished ) {
        fprintf( stderr, "Got %d from goo_sysexec_lst() call. Terminating.\n", sysexecRC );
        abort();
    }
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

