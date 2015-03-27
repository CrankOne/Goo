# ifndef H_GOO_GDS_ARRAY_H
# define H_GOO_GDS_ARRAY_H

# include "gds/goo_gds_forwards.h"
# include "goo_config.h"
# include "goo_types.h"

# ifdef ENABLE_GDS

struct gds_Arr {
    /* ... */
};

struct gds_Arr *
gds_asc_array_new( struct gds_Parser *,
                   union gds_PairList * );

struct gds_Arr *
gds_array_new( struct gds_Parser *,
               union gds_ExprList * );

struct gds_Pair *
gds_asc_array_pair_new( struct gds_Parser *,
                        struct gds_Literal *,
                        struct gds_Expr * );

struct gds_Pair *
gds_asc_array_pair_new_( struct gds_Parser *,
                         const char *,
                         struct gds_Expr * );

struct gds_Function *
gds_array_subset( struct gds_Parser *,
                  struct gds_Function *,
                  struct gds_Function * );


union gds_PairList *
gds_asc_array_append( struct gds_Parser *,
                      union gds_PairList *,
                      struct gds_Pair * );


# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_ARRAY_H */


