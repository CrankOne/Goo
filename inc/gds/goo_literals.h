# ifndef H_GOO_GDS_LITERALS_H
# define H_GOO_GDS_LITERALS_H

# include "goo_config.h"
# include "goo_types.h"
# include "gds/goo_gds_forwards.h"

# ifdef ENABLE_GDS

/**@file goo_literals.h
 * @brief File contains general parser state machine routines.
 *
 * Parsed literals represents logic, numerical or string immutable
 * data.
 */

# ifdef __cplusplus
extern "C" {
# endif /* __cplusplus */

/**@func interpret_escseq
 * @brief Interprets ANSI escape sequences (like \n, \b, etc.)
 *
 * Returns interpreted sequence as 32-bit length unsigned integer.
 * Implies that byte pointer as a starting character in escaped
 * sequence (first char after back-slash). Parses sequence until
 * EOF or invalid characters. Currently supports octal and hexidecimal
 * encoding and doesn't provide any length check.
 *
 * TODO: length check, wide characters.
 *
 * @param s     a starting encoding byte.
 */
uint32_t interpret_escseq(const char * s);

struct gds_Parser;

struct gds_Literal {
    enum {
        # define declare_enum( num, cname, gname, gdsname ) \
        gdsname ## _CT = num,
        for_all_atomic_datatypes(declare_enum)
        # undef declare_enum
        logic_CT = 0xc,
        string_CT = 0x83,
        struct_CT = 0x84,
    } typecode;
    union {
        # define declare_datfield( num, cname, gname, gdsname ) \
        cname gdsname ## Val;
        for_all_atomic_datatypes(declare_datfield)
        # undef declare_datfield
        uint8_t logicVal;
        char * stringValPtr;
        void * structValPtr;
    } data;
};

struct gds_Literal * interpret_bin_integral(
    struct gds_Parser *,
    const char * );
struct gds_Literal * interpret_oct_integral(
    struct gds_Parser *,
    const char * );
struct gds_Literal * interpret_hex_integral(
    struct gds_Parser *,
    const char * );
struct gds_Literal * interpret_esc_integral(
    struct gds_Parser *,
    const char * );
struct gds_Literal * interpret_dec_integral(
    struct gds_Parser *,
    const char * );


struct gds_Literal * interpret_float_dec(
    struct gds_Parser *,
    const char * );
struct gds_Literal * interpret_float_hex(
    struct gds_Parser *,
    const char * );

struct gds_Literal *
interpret_string_literal(
        struct gds_Parser * P,
        const char * s );

struct gds_Literal *
interpret_logic_true(
        struct gds_Parser * P );
struct gds_Literal *
interpret_logic_false(
        struct gds_Parser * P );


struct gds_Literal *
gds_literal_heapcopy( struct gds_Parser *, struct gds_Literal * );
void
gds_literal_heapfree( struct gds_Parser *, struct gds_Literal * );


# ifdef __cplusplus
}
# endif /* __cplusplus */

# endif  /* ENABLE_GDS */

# endif  /* H_GOO_GDS_LITERALS_H */

