# ifndef H_GOO_VERSIONING_H
# define H_GOO_VERSIONING_H

# include "goo_types.h"

# ifdef __cplusplus
extern "C" {
# endif

struct Goo_Versioning{
    const char * buildTimestamp;
    const char * gitCommitHash;
    const char * gitCommitStr;
    uint32_t encodedFeatures,
             encodedLibsSupport;
} ;

extern const struct Goo_Versioning gooVersioning;

/** Prints build conf to stream. */
void goo_build_info( FILE * );

# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_VERSIONING_H */

