# include <unordered_map>
# include <type_traits>
# include <cstring>
# include "goo_exception.hpp"
# include "goo_types.h"

namespace goo {

# define implement_typeid_getter( num, cnm, hnm, gdsnm ) \
template<> AtomicTypeID get_atomic_typeid< cnm >() { return num; }
for_all_atomic_datatypes(implement_typeid_getter)
# undef implement_typeid_getter

# ifdef ENABLE_GDS

extern "C" {

void gds_parser_raise_error(
        struct GDS_Parser * P,
        const char * filename,
        uint16_t line,
        uint16_t columnBgn,
        uint16_t columnEnd,
        const char * details ) {
    emraise( gdsError,
        "Parser %p at " ESC_CLRGREEN "%s" ESC_CLRCLEAR ":" ESC_BLDYELLOW "%u" ESC_CLRCLEAR ":" ESC_CLRYELLOW "%u-%u" ESC_CLRCLEAR " got an error: " ESC_UNLRED "%s" ESC_CLRCLEAR,
        P, filename, line, columnBgn, columnEnd, details);
}

void gds_parser_warning(
        struct GDS_Parser * P,
        const char * filename,
        uint16_t line,
        uint16_t columnBgn,
        uint16_t columnEnd,
        const char * details ) {
    fprintf( stderr,
        "Parser %p at " ESC_CLRGREEN "%s" ESC_CLRCLEAR ":" ESC_BLDYELLOW "%u" ESC_CLRCLEAR ":" ESC_CLRYELLOW "%u-%u" ESC_CLRCLEAR " warns: " ESC_UNLYELLOW "%s" ESC_CLRCLEAR,
        P, filename, line, columnBgn, columnEnd, details);
}

}  // extern "C"

# endif

// Parser error

}  // namespace goo

