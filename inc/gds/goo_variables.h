# ifndef H_GOO_GDS_VARIABLES_H
# define H_GOO_GDS_VARIABLES_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

union gds_VarList * gds_variable_spec_type_for(
    struct gds_Parser *,
    struct gds_TypeID *,
    union gds_VarList * );

union gds_VarList * gds_variable_init_list(
    struct gds_Parser *,
    union gds_VarList *,
    union gds_ExprList *
    );

struct gds_Expr * gds_expr_from_var_decls( struct gds_Parser * P, union gds_VarList * vl );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_VARIABLES_H */

