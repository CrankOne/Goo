
typedef GDS_EXPRESSION_DESCRIPTOR_TYPE gds_ExprDescrT;

# define for_all_gds_expression_types \
    m( Scope         ) \
    m( MathFunction  ) \
    m( LogicFunction ) \
    m( List          ) \
    /* ... */

struct gds_Expression {
    gds_ExprDescrT descriptor;
    union {
    }_;
};

# define declare_allocator_function( structName )                              \
    struct gds_Expression * gds_allocate_ ## structName( struct gds_Parser * );
for_all_gds_expression_types(declare_allocator_function)
# undef declare_allocator_function


struct gds_Scope {
    void * dict;  /* std::map<str-hashsum, gds_Expression *> */
};


void emplace_expression_to_current_scope( struct gds_Parser *,
                                          char *,
                                          struct gds_Expression * );




struct gds_Parser {
    /* ... */
};

