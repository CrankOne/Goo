# include "gds/goo_interpreter.h"
# include "gds/goo_variables.h"

struct gds_VarList *
gds_variable_spec_type_for(
        struct gds_Parser * P,
        struct gds_TypeID * T,
        struct gds_VarList * vl
    ) {
    /* TODO */
    return NULL;
}

struct gds_VarList *
gds_variable_init_list(
        struct gds_Parser * P,
        struct gds_VarList * vl,
        struct gds_ExprList * el
    ) {
    /* TODO */
    return NULL;
}

struct gds_Expr *
gds_expr_from_var_decls(
        struct gds_Parser * P,
        struct gds_VarList * vl ) {
    /* TODO: here we should produce expression from variable
     *       declarations. */
    return NULL;
}

