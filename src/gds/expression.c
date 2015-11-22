# include "gds/expression.h"

/*
 * Arithmetic constant ********************************************************
 */

# define implement_initializer_function( codeN, cTypeName, gooTypeName, gdsTypeName )  \
void gds_init_arithmetic_cst_ ## gdsTypeName ( struct ArithmeticConstant * n, cTypeName value ) { \
    n->type = codeN ; n->value. gooTypeName ## Val = value; }
for_all_atomic_datatypes( implement_initializer_function )
# undef implement_initializer_function

/*
 * Binary operators ***********************************************************
 */

# define define_binary_arithmetical_code( sign, code, name ) \
const BinaryArithOpCode GDS_e_binary_ ## name = code;
for_each_binary_arithmetic_lexical_operator( define_binary_arithmetical_code )
# undef define_binary_arithmetical_code

# define implement_initializer_function( str, code, name )                \
void gds_init_binary_ ## name( struct BinaryArithmeticOperator * n,     \
                               struct GDSExpression * leftOperand,      \
                               struct GDSExpression * rightOperand ) {  \
    n->type = code; n-> left = leftOperand; n-> right = rightOperand; }
for_each_binary_arithmetic_lexical_operator( implement_initializer_function )
# undef implement_initializer_function

