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

struct gds_Expr * gds_expression_from_literal( struct gds_Literal * );
struct gds_Expr * gds_expression_from_math_expr( struct gds_Function * );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_EXPRESSIONS_H */


