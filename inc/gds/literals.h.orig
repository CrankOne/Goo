/**\file goo_literals.h
 * \brief File contains declarations of literals parsing functions.
 *
 * Parsed literals represents logic, numerical or string data --- all
 * the immutable GDS grammar literals. */

# ifndef H_GOO_GDS_LITERALS_H
# define H_GOO_GDS_LITERALS_H

# include "goo_config.h"
# include "goo_types.h"
# include "expression.h"

# ifdef ENABLE_GDS
# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

/**\func interpret_escseq
 * \brief Interprets ANSI escape sequences (like \n, \b, etc.)
 *
 * Returns interpreted sequence as 32-bit length unsigned integer.
 * Implies that byte pointer as a starting character in escaped
 * sequence (first char after back-slash). Parses sequence until
 * EOF or invalid characters. Currently supports octal and hexidecimal
 * encoding.
 *
 * TODO: length check, wide characters.
 *
 * \param s     a starting encoding byte.
 */ uint32_t
gds_interpret_escseq(const char * s);

/**\func gds_interpr_integral_literal
 * \brief Parses integral literal for bases 2/8/10/16
 *
 * Internally, uses strotoi-family functions (including stroll, etc.).
 * For binary base numbers uses custom routine.
 *
 * \returns -1 when 128-bytes long type is requested, but interpreter doesn't
 * support it.
 * \returns -2 on out-of-range error.
 * \returns -3 if no digits where found.
 * \returns 1 if there are extra symbols on tail.
 */ int
gds_interpr_integral_literal(
        struct ArithmeticConstant * A,
        const char * s,
        const uint8_t base
        );

/**\func gds_interpr_esc_integral
 * \brief Parses integral literal provided in form of escape sequence.
 *
 * TODO: more doc.
 */ int
gds_interpr_esc_integral(
    struct ArithmeticConstant *,
    const char * );

/**\func gds_interpr_float_literal
 * \brief Parses integral literal for bases 10/16
 *
 * Internally, uses strotod-family functions (including strof, etc.).
 * Base argument is currently unused.
 *
 * TODO: more checks.
 */ int
gds_interpr_float_literal(
        struct ArithmeticConstant * A,
        const char * s,
        uint8_t base );

# ifdef __cplusplus
}
# endif /* __cplusplus */
# endif  /* ENABLE_GDS */
# endif  /* H_GOO_GDS_LITERALS_H */

