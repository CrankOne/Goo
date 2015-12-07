# include <unordered_map>
# include <type_traits>
# include <cstring>
# include "goo_exception.hpp"
# include "goo_types.h"
# ifdef ENABLE_GDS
# include "gds/interpreter.h"
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
    emraise( gdsError, "Parser %p / %s", P, details);
}

}  // extern "C"

# endif

// Parser error

}  // namespace goo

