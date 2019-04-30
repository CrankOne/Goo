//# include <cassert>
//# include <bitset>
# include <mutex>
//# include <thread>
//# include <vector>
//# include <iostream>
# include <condition_variable>
//# include <chrono>
//# include <typeinfo>
//# include <unordered_map>
//# include <list>

//# include "goo_tsort.tcc"

# include "goo_bitset.hpp"
# include "goo_tsort.tcc"
# include "goo_dataflow/processor.hpp"

# pragma once

namespace goo {
namespace dataflow {

class Worker;
class Framework;

/**@brief Semi-parallel processed DAG synchronization helper.
 * @class TierMonitor
 *
 * Set of processors within executable DAG usually can be sorted out on few
 * groups, by the order of the execution. For example, in graph of three nodes
 * A, B, C, where both B and C depends on A, the possible execution sequences
 * are:
 *  - A, B, C
 *  - A, C, B
 * One can write out this as ({A}, {B,C}) reflecting the fact that B and C
 * can be evaluated in parallel. See more details in DAG documentation.
 *
 * The TierMonitor class offers synchroniation ...
 * */
class Tier {
private:
    std::mutex _accessMtx;
    std::condition_variable _cv;
    Bitset _freeFlags
         , _stateless;

    std::vector<dag::Node<iProcessor>*> _nodes;
protected:
    Tier( std::unordered_set<dag::DAGNode*> & );
    /// Sets n-th processor free indicator bit and notifies all subscribed
    /// worker threads.
    void set_free( size_t n );
    /// Blocks execution of current thread until one of the given will become
    /// available.
    size_t borrow_one( const Bitset &, dag::Node<iProcessor> *& );

    friend class ::goo::dataflow::Worker;
    friend class ::goo::dataflow::Framework;
};

}  // namespace ::goo::dataflow
}  // namespace goo

