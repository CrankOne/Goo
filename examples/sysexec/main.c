# include <goo_utility.h>
# include <stdlib.h>
# include <stdio.h>

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
    struct SysExecArgument * itA;
    struct SysExecEnvVar * itV;
    for( itA = args; itA && itA->argName; ++itA ) {
        itA->nextArg = (itA+1);
    }
    for( itV = envVars; itV && itV->envVarName; ++itV ) {
        itV->nextVar = (itV+1);
    }
    //sysexec_lst( "one", NULL, NULL, 1 );
    return sysexec_lst("./2call.sh", args, envVars, 1);
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
    {
        struct SysExecArgument args[] = {
            { "--arg1", NULL },
            { "one", NULL },
            //{ NULL, NULL }
        };
        if( 2 != (rc = invrun( args, NULL )) ) {
            fprintf( stderr, "Got return code %d from invokation. Expected 2.\n", rc );
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

