# include <unordered_map>
# include <type_traits>
# include <cstring>
# include "goo_types.h"

namespace goo {

# define implement_typeid_getter( num, cnm, hnm, gdsnm ) \
template<> AtomicTypeID get_atomic_typeid< cnm >() { return num; }
for_all_atomic_datatypes(implement_typeid_getter)
# undef implement_typeid_getter

}  // namespace goo

