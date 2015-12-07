# include "goo_config.h"

# ifdef ENABLE_GDS

# define GDS_PARAMETER__DIAGNOSTIC_MESSAGE_BUFFER_LENGTH

# include "gds/diagnostic.h"

void gds_diagnostic_message(
        struct gds_Parser * p,
        enum GDS_DiagnosticStatus status,
        const char * fmt, ... ) {
    char buffer[GDS_PARAMETER__DIAGNOSTIC_MESSAGE_BUFFER_LENGTH];
    // ...
}

void gds_abort( struct gds_Parser * P, int rc ) {
}

# endif  /* ENABLE_GDS */

