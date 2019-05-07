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
# pragma once

# include "goo_tsort.tcc"
# include "goo_dataflow/processor.hpp"

namespace goo {
namespace dataflow {

class Storage;
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
class Tier : public std::vector<dag::Node<iProcessor>*> {
private:
    std::mutex _accessMtx;
    std::condition_variable _cv;
    Bitset _freeFlags
         , _stateless;
protected:
    Tier( std::unordered_set<dag::DAGNode*> & );
    /// Sets n-th processor free indicator bit and notifies all subscribed
    /// worker threads.
    void set_free( size_t n );
    /// Blocks execution of current thread until one of the given will become
    /// available.
    size_t borrow_one( const Bitset &, dag::Node<iProcessor> *& );

    friend class Worker;
    friend class Framework;
    friend class Storage;
};

}  // namespace ::goo::dataflow
}  // namespace goo

