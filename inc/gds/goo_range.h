# ifndef H_GOO_GDS_RANGE_H
# define H_GOO_GDS_RANGE_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

struct gds_Range {
    struct gds_Function * beginTerm;
    struct gds_Function * stepTerm;
    struct gds_Function * endTerm;
};

struct gds_Range *
gds_range_new( struct gds_Parser *,
               struct gds_Function *,
               struct gds_Function *, /* Can be NULL */
               struct gds_Function *
             );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_RANGE_H */


