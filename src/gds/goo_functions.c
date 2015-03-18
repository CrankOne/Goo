# include "gds/goo_functions.h"
# include "gds/goo_interpreter.h"
# include <stdio.h>

# ifdef ENABLE_GDS

/*
 * Math function
 */

# define for_all_gds_math_node_descriptor_postfix( m )  \
    m( gds_math_nodeNumeric,    0x2 )                   \
    m( gds_math_nodeLocVar,     0x1 )                   \
    m( gds_math_nodeMathOp,     0x3 )                   \
    m( gds_math_uDefinedFunc,   0xc0 )

# define for_all_gds_math_node_masks( m )           \
    m( gds_math_nodeTypeMask,   0x3  )              \
    m( gds_math_nodeMathOpMask, 0x3c )

# define for_all_gds_binary_math_operators( m )     \
    m( gds_math_summation_c        , 0x03, '+' )    \
    m( gds_math_subtraction_c      , 0x07, '-' )    \
    m( gds_math_mutliplication_c   , 0x0b, '*' )    \
    m( gds_math_division_c         , 0x0f, '/' )    \
    m( gds_math_power_c            , 0x13, '^' )    \
    m( gds_math_modulo_c           , 0x17, '%' )    \
    m( gds_math_dot_c              , 0x23, '.' )

static const uint8_t
# define declare_const_b( name, code ) name = code,
    for_all_gds_math_node_descriptor_postfix(declare_const_b)
    for_all_gds_math_node_masks(declare_const_b)
# undef declare_const_b
# define declare_const_b( name, code, symbol ) name = code,
    for_all_gds_binary_math_operators(declare_const_b)
# undef declare_const_b
    gds_math_unegotiation_c     = 0x1f;

void gds_math_function_init(
        struct gds_Parser * P,
        struct gds_Function * f,
        struct gds_Function * body,
        const char * name,
        struct gds_ArgList * arglist) {
    printf( "XXX: new function \"%s\" at %p on parser %p with args %p\n",
            name,
            f,
            P,
            arglist);
    f->descriptor = gds_math_uDefinedFunc;
    f->content.asFunction.f = body; /* can be NULL */
    f->content.asFunction.arglist = arglist;
    gds_hashtable_insert( P->thisModule.functions,
                          name,
                          f );
}

struct gds_Function *
gds_math_new_func_from_const(
        struct gds_Parser * P,
        struct gds_Literal * constVal ) {
    struct gds_Function * node = gds_parser_new_Function(P);

    node->descriptor = gds_math_nodeNumeric;
    node->content.asValue = constVal;

    return node;
}

struct gds_Function *
gds_math_new_func_from_locvar(
        struct gds_Parser * P,
        uint8_t lvno ) {
    struct gds_Function * node = gds_parser_new_Function(P);

    node->descriptor = gds_math_nodeLocVar;
    node->content.asLocalVariable.orderNum = lvno;

    return node;
}

struct gds_Function *
gds_math(
        struct gds_Parser * P,
        char opType,
        struct gds_Function * l,
        struct gds_Function * r) {
    struct gds_Function * node = gds_parser_new_Function(P);
    node->content.asMathOperation.l = l;
    node->content.asMathOperation.r = r;
    switch( opType ) {
        # define case_( name, code, glyph ) \
            case glyph : {                  \
                node->descriptor = name;    \
            } break;
        for_all_gds_binary_math_operators(case_);
        # undef case_
        default: {
            fprintf( stderr,
                    "Unknown symbol '%c' interpreted as math operator: internal parser error.\n",
                    opType );
        }
    }; /* switch( opType ) */
    printf( "XXX New math operation 0x%0x(%c) at %p: %p %p.\n",
            node->descriptor,
            opType,
            node,
            l, r);
    return node;
}

struct gds_Function *
gds_math_negotiate(
        struct gds_Parser * P,
        struct gds_Function * o) {
    struct gds_Function * node = gds_parser_new_Function(P);
    node->descriptor = gds_math_unegotiation_c;
    node->content.asMathOperation.l = o;
    node->content.asMathOperation.r = NULL;
    return node;
}

static void _static_print_out_math_expr_graph(
            struct gds_Parser * P,
            struct gds_Function * f ) {
    uint8_t typePart = gds_math_nodeTypeMask & f->descriptor;
    if( typePart == gds_math_nodeNumeric ) {
        printf( "%p -- numeric node %p\n", f, f->content.asValue );
    } else if( typePart == gds_math_nodeLocVar ) {
        printf( "%p -- locvar node \"%s\"\n", f, f->content.asLocalVariable.name );
    } else if( typePart == gds_math_nodeMathOp ) {
        printf( "%p -- math op node ", f );
        uint8_t mathOpPart = (gds_math_nodeTypeMask | gds_math_nodeMathOpMask) & f->descriptor;
        switch( mathOpPart ) {
        # define case_( name, code, glyph ) \
            case code : {                   \
                printf( "%c: %p %p\n",      \
                        glyph,              \
                        f->content.asMathOperation.l,  \
                        f->content.asMathOperation.r); \
                _static_print_out_math_expr_graph( P, f->content.asMathOperation.l ); \
                _static_print_out_math_expr_graph( P, f->content.asMathOperation.r ); \
            } break;
        for_all_gds_binary_math_operators(case_);
        # undef case_
        default: {
            gds_error(P, "logic error #2");
        }
    }; /* switch( mathOpPart ) */
    } else {
        gds_error( P, "logic error #1." );
    }
}

void gds_math_function_resolve(
        struct gds_Parser * P,
        struct gds_Function * f) {
    struct gds_ArgList * c = f->content.asFunction.arglist;
    printf( "Function %p of parser %p is ready to be resolved:\n", f, P );
    printf( "    - arglist %p:\n", c );
    if( c->identifier ) do {
        printf( "        %s\n", c->identifier );
        c = c->next;
    } while( c );
    _static_print_out_math_expr_graph( P, f->content.asFunction.f );
}

# endif  /* ENABLE_GDS */

