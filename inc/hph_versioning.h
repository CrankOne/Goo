# ifndef H_HPH_VERSIONING_H
# define H_HPH_VERSIONING_H

# include "hph_types.h"

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

/** Prints build conf to stdout. */
void build_info();

# ifdef __cplusplus
}
# endif

# endif  /* H_HPH_VERSIONING_H */

