# ifndef H_GOO_GDS_VARIABLES_H
# define H_GOO_GDS_VARIABLES_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

struct gds_VarList {
    const char * identifier;
    struct gds_VarList * next;
};

struct gds_VarList * gds_variable_spec_type_for(
    struct gds_Parser *,
    struct gds_TypeID *,
    struct gds_VarList * );

struct gds_VarList * gds_variable_init_list(
    struct gds_Parser *,
    struct gds_VarList *,
    struct gds_ExprList *
    );

struct gds_Expr * gds_expr_from_var_decls( struct gds_Parser * P, struct gds_VarList * vl );

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_VARIABLES_H */

