# include <cstring>
# include <unordered_map>
# include <algorithm>
# include <iostream> // XXX
# include "hph_exception.hpp"
# include "hph_allocator.hpp"

# ifdef DEBUG_ALLOCATOR
# include <ext/debug_allocator.h>
# endif

namespace hph {
namespace mem {

# ifndef DEBUG_ALLOCATOR
    typedef STLAllocator<std::allocator> DefaultAllocatorType;
# else
template<typename T>
struct DebugAllocatorProxy : public __gnu_cxx::debug_allocator<std::allocator<T> > {
};
    typedef STLAllocator<DebugAllocatorProxy> DefaultAllocatorType;
# endif

typedef DefaultAllocatorType EmergencyAllocatorType;  // TODO !!!

static DefaultAllocatorType *   __defaultAllocator;
static EmergencyAllocatorType * __emergencyAllocator;

iAllocator *
get_default_allocator() {
    return __defaultAllocator;
}

static void __construct_default_allocator() __attribute__((__constructor__(155)));
static void __delete_default_allocator() __attribute__((__destructor__(155)));

static void __construct_default_allocator() {
    __defaultAllocator    = new DefaultAllocatorType();
    __emergencyAllocator  = new EmergencyAllocatorType();
}

static void __delete_default_allocator() {
    delete __defaultAllocator;
    delete __emergencyAllocator;
}

}  // namespace mem
}  // namespace hph

