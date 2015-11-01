# ifndef H_GOO_VERSIONING_H
# define H_GOO_VERSIONING_H

# include "goo_types.h"

# ifdef __cplusplus
extern "C" {
# endif

struct HPH_Versioning{
    const char * buildTimestamp;
    const char * gitCommitHash;
    const char * gitCommitStr;
    uint32_t encodedFeatures,
             encodedLibsSupport;
} ;

extern const struct HPH_Versioning hphVersioning;

/** Prints build conf to stream. */
void build_info( FILE * );

# ifdef __cplusplus
}
# endif

# endif  /* H_GOO_VERSIONING_H */

