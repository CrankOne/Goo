# include <stdarg.h>
# include <string.h>
# include <stdio.h>
# include "goo_config.h"
# include "goo_utility.h"
# include "gds/interpreter.h"

# ifdef ENABLE_GDS

# define GDS_PARAMETER__DIAGNOSTIC_MESSAGE_BUFFER_LENGTH 512

# include "gds/diagnostic.h"

void gds_diagnostic_message(
        struct gds_Parser * p,
        enum GDS_DiagnosticStatus status,
        const char * fmt_, ... ) {
    char fmtBf[  GDS_PARAMETER__DIAGNOSTIC_MESSAGE_BUFFER_LENGTH],
         outBf[2*GDS_PARAMETER__DIAGNOSTIC_MESSAGE_BUFFER_LENGTH];
    int rc = 0;
    va_list argp;
    
    if( strstr( fmt_, "${location}" ) != NULL ) {
        /* TODO: subst location */
        rc = replace_string( fmt_, "${location}", "TODO", fmtBf, sizeof(fmtBf) );
        if( rc < 0 ) {
            gds_error( p, "Diagnostic message substitution error." );
            gds_abort( p, -1 );
        }
    }
    /* ... */
    vsnprintf( outBf, sizeof(outBf), fmtBf, argp );
    switch( status ) {
        case( gds_kDiagnostic_warning ) : {
            fputs( outBf, p->errStream );
        } break;
        case( gds_kDiagnostic_error ) :
        default:
            fputs( outBf, p->errStream );
            gds_abort(p, -2);
    };
}

void gds_abort( struct gds_Parser * P, int rc ) {
}

# endif  /* ENABLE_GDS */

