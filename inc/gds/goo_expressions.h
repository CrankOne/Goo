# ifndef H_GOO_GDS_EXPRESSIONS_H
# define H_GOO_GDS_EXPRESSIONS_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

# define for_each_gds_expression_type(m)                            \
    m( function,       0x1,    "Expression without side effects." ) \
    m( variable,       0x2,     "..." )

struct gds_Expr {
    uint32_t descriptor;
    union {
        /* TODO */
    } content;
};

struct gds_Expr * gds_expression_from_literal( struct gds_Literal * );
struct gds_Expr * gds_expression_from_math_expr( struct gds_Function * );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_EXPRESSIONS_H */


