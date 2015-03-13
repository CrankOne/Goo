# include <unordered_map>
# include <type_traits>
# include <cstring>
# include "goo_exception.hpp"
# include "goo_types.h"
# ifdef ENABLE_GDS
# include "gds/goo_interpreter.h"
# endif

namespace goo {

# define implement_typeid_getter( num, cnm, hnm, gdsnm ) \
template<> AtomicTypeID get_atomic_typeid< cnm >() { return num; }
for_all_atomic_datatypes(implement_typeid_getter)
# undef implement_typeid_getter

# ifdef ENABLE_GDS

extern "C" {

void gds_error(
        struct gds_Parser * P,
        const char * details ) {
    emraise( gdsError,
        "Parser %p at " ESC_CLRGREEN "%s" ESC_CLRCLEAR ":" ESC_BLDYELLOW "%u" ESC_CLRCLEAR ":" ESC_CLRYELLOW "%u-%u" ESC_CLRCLEAR " got an error: " ESC_UNLRED "%s" ESC_CLRCLEAR,
        P,
        P->currentFilename,
        P->location.first_line,
        P->location.first_column,
        P->location.last_column,
        details);
}

void gds_warn(
        struct gds_Parser * P,
        const char * details ) {
    fprintf( stderr,
        "Parser %p at " ESC_CLRGREEN "%s" ESC_CLRCLEAR ":" ESC_BLDYELLOW "%u" ESC_CLRCLEAR ":" ESC_CLRYELLOW "%u-%u" ESC_CLRCLEAR " warns: " ESC_UNLYELLOW "%s" ESC_CLRCLEAR,
        P,
        P->currentFilename,
        P->location.first_line,
        P->location.first_column,
        P->location.last_column,
        details);
}

}  // extern "C"

# endif

// Parser error

}  // namespace goo

