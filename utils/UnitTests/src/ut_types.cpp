# include <iostream>
# include <iomanip>
# include "utest.hpp"

# if 0
HPH_UT_BGN( Types ) {

out << std::setiosflags(std::ios::fixed)
    << std::setw(4) << "#"
    << std::setw(15) << "C-Name"
    << std::setw(15) << "HPH API name"
    << std::setw(15) << "HDS name"
    << std::setw(6)  << "size"
    << std::setw(6)  << "int"
    << std::setw(6)  << "signd"
    << std::setw(6)  << "flt.p"
    << std::setw(6)  << "comp"
    << std::endl;

# define dump_type_info( nm, ct, hnm, hdsnm ) \
out << std::setw(4)  << #nm \
    << std::setw(15) << #ct \
    << std::setw(15) << #hnm \
    << std::setw(15) << #hdsnm \
    << std::setw(6)  << (IS_COMPOUND_TYPE(nm) ? 0 : sizeof(ct)) \
    << std::setw(6)  << IS_INTEGRAL_TYPE(nm) \
    << std::setw(6)  << IS_SIGNED_INTEGRAL_TYPE(nm) \
    << std::setw(6)  << IS_FLOATING_POINT_TYPE(nm) \
    << std::setw(6)  << IS_COMPOUND_TYPE(nm) \
    << std::endl;
for_all_atomic_datatypes(dump_type_info)
# undef dump_type_info

//_ASSERT(0, "artificial");

} HPH_UT_END( Types )

# endif

