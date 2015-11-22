# ifndef H_GOO_GDS_EXPR_NODE_H
# define H_GOO_GDS_EXPR_NODE_H

# include "goo_types.h"

struct GDSExpression;

/*
 * Arithmetic constant ********************************************************
 */
struct ArithmeticConstant {
    TypeCode type;
    union ArithmeticValue {
        # define declare_enum_field( code, cTypeName, gooName, gdsName ) \
        cTypeName gooName ## Val;
        for_all_atomic_datatypes( declare_enum_field );
        # undef declare_enum_field
    } value;
};

# define declare_initializer_function( code, cTypeName, gooTypeName, gdsTypeName ) \
void gds_init_arithmetic_cst_ ## gdsTypeName ( struct ArithmeticConstant *, cTypeName value );
for_all_atomic_datatypes( declare_initializer_function )
# undef declare_initializer_function



/*
 * Binary operators ***********************************************************
 */
# define for_each_binary_arithmetic_lexical_operator( m )   \
    m( "+",             0x1,    summation )                 \
    m( "-",             0x2,    subtraction )               \
    m( "*",             0x3,    product )                   \
    m( "/",             0x4,    division )                  \
    m( "^",             0x5,    exponentiation )            \
    m( "%%",            0x6,    divRemainder )              \
    /* ... */

typedef uint8_t BinaryArithOpCode;

# define declare_binary_arithmetical_code( sign, code, name ) \
    extern const BinaryArithOpCode GDS_e_binary_ ## name;
for_each_binary_arithmetic_lexical_operator( declare_binary_arithmetical_code )
# undef declare_binary_arithmetical_code

struct BinaryArithmeticOperator {
    BinaryArithOpCode type;
    struct GDSExpression * left,
                         * right
                         ;
};

# define declare_initializer_function( str, code, name )            \
void gds_init_binary_ ## name( struct BinaryArithmeticOperator *,   \
                               struct GDSExpression *,              \
                               struct GDSExpression *);
for_each_binary_arithmetic_lexical_operator( declare_initializer_function )
# undef declare_initializer_function


/*
 * Expression structure *******************************************************
 */
# define for_each_gds_expr_node_type( m )                           \
    m( unresolved,      0x0,    "Unresolved expression operand." )  \
    m( arithmConstant,  0x2,    "GDS numeric expression operand." ) \
    m( lexicalOperator, 0x3,    "GDS lexical operator." )           \
    m( function,        0x4,    "GDS functional expression." )      \
    m( scope,           0x5,    "Scope / associative array." )      \
    /* ... */

struct GDSExpression {
    enum GDS_ExpressionType {
        # define declare_enum_field( name, code, description ) \
            gdsE_ ## name = code,
        for_each_gds_expr_node_type( declare_enum_field )
        # undef declare_enum_field
    } expressionType;
    union UData {
        struct ArithmeticConstant * arithmetic;
        struct BinaryArithmeticOperator * binop;
        /* ... other types ... */
    } pointer;
};

# endif  /* H_GOO_GDS_EXPR_NODE_H */

