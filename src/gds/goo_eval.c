
/*
 *
 * # Bits               Hex code    Meaning
 * xxxx xxxx xxxx xxxx
 * 0000 0000 0000 0000  0x0000      Undefined -- Reserved for sentinels.
 * 0000 0111 0000 0000  0x????      Plurality mask.
 * 0000 1000 0000 0000  0x????      `temporary' bit -- means that node should be resolved in some way.
 * xxxx x000 xxxx xxxx  0x????      Scalar object.
 * xxxx x001 xxxx xxxx  0x????      List.
 * xxxx x101 xxxx xxxx  0x????      Generator.
 * xxxx x100 xxxx xxxx  0x????      Discrete injection (map).
 * xxxx x110 xxxx xxxx  0x????      Discrete bijection (bimap).
 * xxxx x111 xxxx xxxx  0x????      Table.
 * 0xxx xxyy xxxx xxxx  0x????      Non-evaluable value -- resolved variable or constant.
 * 01x0 xxyy xxxx zzzz  0x????          Numerical constant.
 * 0100 xxyy xxxx zzzz  0x????              Integral numeric constant.
 * 0100 xxyy xxxx xxx1  0x????              (Integral `signed' bit)
 * 0100 xxyy xxxx 0000  0x????              1-byte unsigned integer (C's unsigned char)
 * 0100 xxyy xxxx 0001  0x????              1-byte          integer (C's          char)
 * 0100 xxyy xxxx 0010  0x????              2-byte unsigned integer (C's unsigned short)
 * 0100 xxyy xxxx 0011  0x????              2-byte          integer (C's          short)
 * 0100 xxyy xxxx 0100  0x????              4-byte unsigned integer (C's unsigned int)
 * 0100 xxyy xxxx 0101  0x????              4-byte          integer (C's          int)
 * 0100 xxyy xxxx 0110  0x????              8-byte unsigned integer (C's unsigned long int)
 * 0100 xxyy xxxx 0111  0x????              8-byte          integer (C's          long int)
 * 0100 xxyy xxxx 1000  0x????             16-byte unsigned integer (C's unsigned long long int)
 * 0100 xxyy xxxx 1001  0x????             16-byte          integer (C's          long long int)
 * 0100 xxyy xxxx 1010  0x????             32-byte unsigned integer (C's unsigned long long long int -- reserved)
 * 0100 xxyy xxxx 1011  0x????             32-byte          integer (C's          long long long int -- reserved)
 * 0110 xxyy xxxx 00zz  0x????              Float numeric constant.
 * 0110 xxyy xxxx 0000  0x????                  4-bytes float numconst (C's float)
 * 0110 xxyy xxxx 0001  0x????                  8-bytes float numconst (C's double)
 * 0110 xxyy xxxx 0010  0x????                 16-bytes float numconst (C's long double)
 * 0110 xxyy xxxx 0011  0x????                 32-bytes float numconst (C's long long double -- reserved)
 * 0001 xxyy xxxx xxxz  0x????          Logical constant.
 * 0001 xxyy xxxx xxx0  0x????              FALSE
 * 0001 xxyy xxxx xxx1  0x????              TRUE
 * 1xxx xxyy xxxx xxxx  0x????      Evaluable node.
 * 1100 xxxx xxxy yyyy  0x????      Arithmetical operations
 * 1100 xxxx xxxy y1yy  0x????      Exponentiation
 * 1100 xxxx xxxy yyy1  0x????          Unary arithmetics:
 * 1100 xxxx xxx0 0001  0x????              unary - (negotiation)
 * 1100 xxxx xxx0 0111  0x????              unary ^(-1) (multiplicative inversion -- division)
 * 1100 xxxx xxx0 yy10  0x????          Binary arithmetics:
 * 1100 xxxx xxx0 1010  0x????              Binary modulo % (division remainder)
 * 1100 xxxx xxx0 0110  0x????              Binary exponentiation ^ (power)
 * 1100 xxxx xxx0 0010  0x????              Binary dot product
 * 1100 xxxx xxxy yy11  0x????          N-ary arithmetics:
 * 1100 xxxx xxx0 0011  0x????              N-ary + (summation)
 * 1100 xxxx xxx0 1011  0x????              N-ary * (multiplication)
 * 1100 xxxx xxx1 1011  0x????              N-ary × (cross product -- reserved)
 * 1101 xyyy yyyy yyyy  0x????      Math-defined (substitutive) function (resolved or not)
 * 1111 xyyy yyyy yyyy  0x????      Numerical (non-substitutive) function (resolved or not)
 */


int
gds_function_eval( struct gds_Parser * P,
                   struct gds_Function *  fr,
                   struct gds_Function ** fl) {

    if( gds_function_is_numvar(fi->descriptor) ) {
        *fl = fr;
    } else if( gds_function_is_locvar(fi->descriptor) ) {
        *fl = gds_evaluation_context_get_variable(
                    gds_parser_top_evaluation_context(P),
                    fr->content.asLocalVariable.orderNum );
    } else {  /* function represents an operation */
        fi = gds_parser_new_function( P );
        if( gds_function_is_math_op(fi) ) {
            /* do the math */
        } else if( gds_function_is_logic_op(fi) ) {
            /* do the logic */
        }
    }

    return 0;
}

