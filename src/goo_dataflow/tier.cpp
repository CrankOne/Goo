# include "goo_dataflow/tier.hpp"

namespace goo {
namespace dataflow {

# if 0
Tier::Tier(size_t n) : _freeFlags(n) {
    if(!_freeFlags.empty()) {
        _freeFlags.set();
    }
}
# endif

Tier::Tier( std::unordered_set<dag::DAGNode*> & ns ) : _freeFlags(ns.size()) {
    assert( !ns.empty() );
    _freeFlags.set();
    // ... TODO: fill vector
}

void
Tier::set_free( size_t n ) {
    std::unique_lock<std::mutex> lock(_accessMtx);
    _freeFlags.set(n);
    _cv.notify_all();
}

size_t
Tier::borrow_one( const Bitset & toProcess ) {
    size_t n;
    std::unique_lock<std::mutex> lock(_accessMtx);
    auto available = toProcess & _freeFlags;
    while( available.none() ) {
        _cv.wait(lock);  // NOTE: frees _accessMtx while waiting
        available = toProcess & _freeFlags;
    }
    // Get first freed processor number, mark it as busy and return number
    for( n = 0; n < available.size(); ++n ) {
        if( available.test(n) ) {
            _freeFlags.reset(n);
            return n;
        }
    }
    assert(false);
}

}  // namespace goo::dataflow
}  // namespace goo
