# include "gds/goo_functions.h"
# include "gds/goo_interpreter.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# ifdef ENABLE_GDS

/*
 * Math function
 */

const uint8_t
    # define declare_const(code, name, descr)   \
    gds_math_ ## name = code,
        for_all_gds_math_1categories( declare_const )
    # undef declare_const

    # define declare_const(name, code, sym)   \
    gds_math_binary_ ## name = code,
        for_all_binary_math( declare_const )
    # undef declare_const

    # define declare_const(name, code, sym)   \
    gds_math_binary_l_ ## name = code,
        for_all_binary_logic(declare_const)
    # undef declare_const

    # define declare_const(name, code ) \
    gds_math_tl_ ## name = code,
        for_all_ternary_logic(declare_const)
    # undef declare_const

    # define declare_const(name, code)  \
    name = code,
    for_all_unclassified_operations(declare_const)
    # undef declare_const

gds_operations_sentinel = 0 ;

void gds_math_function_init(
        struct gds_Parser * P,
        struct gds_Function * f,
        struct gds_Function * body,
        const char * name,
        union gds_ArgList * arglist) {
    /*
    printf( "XXX: new function \"%s\" at %p on parser %p with args %p\n",
            name,
            f,
            P,
            arglist);
    */
    f->descriptor = gds_math_function;
    f->content.asFunction.f = body; /* can be NULL */
    f->content.asFunction.arglist = arglist;
    f->content.asFunction.name = strdup(name);
    gds_hashtable_insert( P->thisModule.functions,
                          name,
                          f );
}

struct gds_Function *
gds_math_new_func_from_const(
        struct gds_Parser * P,
        struct gds_Literal * constVal ) {
    struct gds_Function * node = gds_parser_new_Function(P);

    node->descriptor = gds_math_numVal;
    node->content.asValue = constVal;

    return node;
}

struct gds_Function *
gds_math_new_func_from_locvar(
        struct gds_Parser * P,
        uint8_t lvno ) {
    struct gds_Function * node = gds_parser_new_Function(P);

    node->descriptor = gds_math_locVar;
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
                node->descriptor = gds_math_binary_ ## name;    \
            } break;
        for_all_binary_math(case_);
        # undef case_
        default: {
            fprintf( stderr,
                    "Unknown symbol '%c' interpreted as math operator: internal parser error.\n",
                    opType );
        }
    }; /* switch( opType ) */
    /*
    printf( "XXX New math operation 0x%0x(%c) at %p: %p %p.\n",
            node->descriptor,
            opType,
            node,
            l, r);
    */
    return node;
}

struct gds_Function *
gds_math_logical_binary_op(
        struct gds_Parser * P,
        char opType,
        struct gds_Function * l,
        struct gds_Function * r) {
    struct gds_Function * node = gds_parser_new_Function(P);
    node->content.asLogicalOperation.l = l;
    node->content.asLogicalOperation.r = r;
    switch( opType ) {
        # define case_( name, code, glyph ) \
            case glyph : {                  \
                node->descriptor = gds_math_binary_l_ ## name;    \
            } break;
        for_all_binary_logic(case_);
        # undef case_
        default: {
            fprintf( stderr,
                    "Unknown symbol '%c' interpreted as logical operator: internal parser error.\n",
                    opType );
        }
    }; /* switch( opType ) */
    /*
    printf( "XXX New math operation 0x%0x(%c) at %p: %p %p.\n",
            node->descriptor,
            opType,
            node,
            l, r);
    */
    return node;
}

struct gds_Function *
gds_math_negotiate(
        struct gds_Parser * P,
        struct gds_Function * o) {
    struct gds_Function * node = gds_parser_new_Function(P);
    node->descriptor = gds_math_unary_negotiation;
    node->content.asMathOperation.l = o;
    node->content.asMathOperation.r = NULL;
    return node;
}
# if 0
static void _static_print_out_math_expr_graph(
            struct gds_Parser * P,
            struct gds_Function * f ) {
    uint8_t typePart = gds_math_nodeTypeMask & f->descriptor;
    if( typePart == gds_math_nodeNumeric ) {
        printf( "%p -- numeric node %p\n", f, f->content.asValue );
    } else if( typePart == gds_math_nodeLocVar ) {
        printf( "%p -- locvar node \"%d\"\n", f, f->content.asLocalVariable.orderNum );
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
# endif

struct gds_Function *
gds_function_heapcopy( struct gds_Parser * P, const struct gds_Function * f ) {
    if( !f ) {return NULL;}
    struct gds_Function * cf = gds_heap_alloc( P->literals, sizeof(struct gds_Function) );
    memcpy( cf, f, sizeof(struct gds_Function) );
    uint8_t typePart = gds_math_MASK_postfix & f->descriptor;
    if( typePart == gds_math_numVal ) {
        cf->content.asValue = 
            gds_literal_heapcopy( P, f->content.asValue );
    } else if( typePart == gds_math_mathOp ) {
        uint8_t mathOpPart = gds_math_MASK_arithm & f->descriptor;
        switch( mathOpPart ) {
            # define case_( name, code, glyph ) \
                case code : {                   \
                    cf->content.asMathOperation.l = gds_function_heapcopy( P, f->content.asMathOperation.l ); \
                    cf->content.asMathOperation.r = gds_function_heapcopy( P, f->content.asMathOperation.r ); \
                } break;
            for_all_binary_math(case_);
            # undef case_
        }; /* switch( mathOpPart ) */
        if(gds_math_unary_negotiation == mathOpPart ) {
            cf->content.asMathOperation.l = gds_function_heapcopy( P, f->content.asMathOperation.l );
        }
    }
    return cf;
}

void
gds_function_heapfree( struct gds_Parser * P, struct gds_Function * f ) {
    if( !f ) {return;}
    uint8_t typePart = gds_math_MASK_postfix & f->descriptor;
    if( typePart == gds_math_numVal ) {
        gds_literal_heapfree( P, f->content.asValue );
    } /*else if( typePart == gds_math_nodeLocVar ) {
        Do nothing for locvar, as its data is stored entirely in current node.
    } */else if( typePart == gds_math_mathOp ) {
        uint8_t mathOpPart = gds_math_MASK_arithm & f->descriptor;
        switch( mathOpPart ) {
            # define case_( name, code, glyph ) \
                case code : {                   \
                    gds_function_heapfree( P, f->content.asMathOperation.l ); \
                    gds_function_heapfree( P, f->content.asMathOperation.r ); \
                } break;
            for_all_binary_math(case_);
            # undef case_
        }; /* switch( mathOpPart ) */
        if(gds_math_unary_negotiation == mathOpPart ) {
            gds_function_heapfree( P, f->content.asMathOperation.l );
        }
    }
    gds_heap_erase(P->literals, f);
}

struct gds_Expr *
gds_expr_from_func_decl(
        struct gds_Parser * P,
        struct gds_Function * f ) {
    /* TODO: here we should make expression from function,
     * that currently aren't necessary. */
    return NULL;
}

# endif  /* ENABLE_GDS */

