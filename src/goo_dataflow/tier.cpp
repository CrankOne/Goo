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

Tier::Tier( std::unordered_set<dag::DAGNode*> & ns ) : _freeFlags(ns.size())
                                                     , _stateless(ns.size()) {
    assert( !ns.empty() );
    _freeFlags.set();
    for( auto nodePtr : ns ) {
        _nodes.push_back( static_cast<dag::Node<iProcessor>*>(nodePtr) );
    }
    // TODO: stateless processors have to be considered as "always free" ones
    // since there is nothing to guard from concurent access. On the loop above
    // we have to check this property of processor, and add set the proper bit
    // in `_stateless' bitset.
    _stateless.reset();
}

void
Tier::set_free( size_t n ) {
    std::unique_lock<std::mutex> lock(_accessMtx);
    _freeFlags.set(n);
    _cv.notify_all();
}

size_t
Tier::borrow_one( const Bitset & toProcess
                , dag::Node<iProcessor> *& dest ) {
    assert(toProcess);
    size_t n;
    std::unique_lock<std::mutex> lock(_accessMtx);
    // Hang on CV till one of the nodes in tier become available.
    auto available = toProcess & (_freeFlags | _stateless);
    for( ; available.none()
         ; available = toProcess & (_freeFlags | _stateless) ) {
        _cv.wait(lock);  // NOTE: frees _accessMtx while waiting
    }
    // Get first freed processor number, mark it as busy and return number
    for( n = 0; n < available.size(); ++n ) {
        if( available.test(n) ) {
            dest = _nodes[n];
            _freeFlags.reset(n);
            return n;
        }
    }
    // This section of code shall never be reached since we assume that there
    // is at least one free processor is free. If we came here it most probably
    // means corruption/malfunction of the _freeFlags bitset.
    emraise( badState, "Dataflow DAG's tier monitoring bitset malfunction." )
}

}  // namespace goo::dataflow
}  // namespace goo
