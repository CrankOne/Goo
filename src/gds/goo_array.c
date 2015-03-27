# include "gds/goo_array.h"
# include <stdlib.h>

# ifdef ENABLE_GDS

struct gds_Arr *
gds_asc_array_new( struct gds_Parser * P,
                   union gds_PairList * pl ) {
    /* TODO */
    return NULL;
}

struct gds_Arr *
gds_array_new( struct gds_Parser * P,
               union gds_ExprList * el ) {
    /* TODO */
    return NULL;
}

struct gds_Pair *
gds_asc_array_pair_new( struct gds_Parser * P,
                        struct gds_Literal * lt,
                        struct gds_Expr * e ) {
    /* TODO */
    return NULL;
}

struct gds_Pair *
gds_asc_array_pair_new_( struct gds_Parser * P,
                         const char * lt,
                         struct gds_Expr * e) {
    /* TODO */
    return NULL;
}

struct gds_Function *
gds_array_subset( struct gds_Parser * P,
                  struct gds_Function * arr,
                  struct gds_Function * ix ) {
    /* TODO */
    return NULL;
}


union gds_PairList *
gds_asc_array_append( struct gds_Parser * P,
                      union gds_PairList * pl,
                      struct gds_Pair * pair ) {
    /* TODO */
    return NULL;
}

# endif  /*ENABLE_GDS*/

