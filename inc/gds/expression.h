/**\file
 * \ingroup GDS
 *
 * GDS generic expression data type representation.
 * */

# ifndef H_GOO_GDS_EXPR_NODE_H
# define H_GOO_GDS_EXPR_NODE_H

# include "goo_types.h"

/**\defgroup GDS
 *
 * @{ */

struct GDSExpression;

/*
 * Arithmetic constant ********************************************************
 */
/**\brief Numerical value representation in GDS.
 * \see GDSExpression */
struct ArithmeticConstant {
    TypeCode type;
    union ArithmeticValue {
        # define declare_enum_field( code, cTypeName, gooName, gdsName ) \
        cTypeName gooName ## Val;
        for_all_atomic_datatypes( declare_enum_field );
        # undef declare_enum_field
    } value;
};


/**\brief Initializes binary arithmetic expression node. */
struct GDSExpression * gds_init_arithmetic_value_expr(
                TypeCode typeCode,
                struct GDSExpression * tNode,
                union ArithmeticValue value);

/*
 * Binary operators ***********************************************************
 */
/**\def for_each_binary_arithmetic_lexical_operator
 * \ingroup xmacro GDS-simple-arithmetics
 * \brief Binary arithmetic operators table.
 *
 * Enlists supported binary operators.
 * */
# define for_each_binary_arithmetic_lexical_operator( m )   \
    m( "+",             0x1,    summation )                 \
    m( "-",             0x2,    subtraction )               \
    m( "*",             0x3,    production )                \
    m( "/",             0x4,    division )                  \
    m( "^",             0x5,    exponentiation )            \
    m( "%%",            0x6,    divRemainder )              \
    /* ... */

typedef uint8_t BinaryArithOpCode;

# define EX_declare_binary_arithmetical_code( sign, code, name ) \
    extern const BinaryArithOpCode GDS_e_binary_ ## name;
for_each_binary_arithmetic_lexical_operator( EX_declare_binary_arithmetical_code )
# undef EX_declare_binary_arithmetical_code

/**\brief Type of binary operator calculator callback function. */
typedef TypeCode (*SimpleArithmeticBinaryOperator)(
        struct ArithmeticConstant *,
        struct ArithmeticConstant *,
        struct ArithmeticConstant *);

/**\struct BinaryArithmeticOperator
 * \brief Binary arithmetic expression node.
 * \ingroup GDS-expression
 *
 * Represents binary arithmetical operator node in GDS expression node.
 * \see GDSExpression */
struct BinaryArithmeticOperator {
    BinaryArithOpCode type;
    union {
        void(*f)();
        SimpleArithmeticBinaryOperator simple_arithmetic_binary_operator;
        /* ... */
    } resolvedFunctionCache;
    struct GDSExpression * left,
                         * right
                         ;
};

/**\brief Initializes binary arithmetic expression node. */
struct GDSExpression * gds_init_binary_operator_expr(
                BinaryArithOpCode binopCode,
                struct GDSExpression * tNode,
                struct GDSExpression * left,
                struct GDSExpression * right);


/*
 * Expression structure *******************************************************
 */
/**\def for_each_gds_expr_node_type
 * \ingroup xmacro GDS-expression
 * \brief GDS expression subtypes table.
 * */
# define for_each_gds_expr_node_type( m )                               \
    m( unresolved,      0x0,    "Unresolved expression operand." )      \
    m( arithmConstant,  0x2,    "GDS numeric expression operand." )     \
    m( binArithOperator,0x3,    "GDS binary arithmetical operator." )   \
    m( function,        0x4,    "GDS functional expression." )          \
    m( scope,           0x5,    "Scope / associative array." )          \
    /* ... */

/**\struct GDSExpression
 * \brief GDS expression node.
 * \ingroup GDS-expression
 *
 * Represents binary arithmetical operator node in GDS expression node.
 * \see ArithmeticConstant
 * \see BinaryArithmeticOperator */
struct GDSExpression {
    /**\brief Expression node descriptor. */
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

/**@}*/

# endif  /* H_GOO_GDS_EXPR_NODE_H */
