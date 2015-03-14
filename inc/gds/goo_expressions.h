# ifndef H_GOO_GDS_EXPRESSIONS_H
# define H_GOO_GDS_EXPRESSIONS_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

struct gds_Expr {
    uint8_t descriptor;
    union {
        /* TODO */
    } content;
};

struct gds_ExprList {
    struct gds_Expr * cexpr;
    struct gds_ExprList * next;
};

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_EXPRESSIONS_H */


