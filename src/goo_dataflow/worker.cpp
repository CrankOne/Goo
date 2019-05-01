# include "goo_dataflow/worker.hpp"
# include "goo_exception.hpp"

namespace goo {
namespace dataflow {

Storage::Storage( const Framework::Cache & fwCache ) {
    dag::Order order = dag::dfs(_nodes);
    std::list<Tier *> tiers;
    for(auto tierNodes : order) {
        tiers.push_back(new Tier(tierNodes));
        for(auto dagNodePtr : tierNodes) {
            auto nodePtr = static_cast<dag::Node<iProcessor *>*>(dagNodePtr);
            iProcessor * p = nodePtr->data();
            for( auto slotPtr : p->slots() ) {
                //slotPtr->name()
                //slotPtr->type()
            }
        }
    }
}

void
Storage::build_variables_map( ValuesMap & vm
                            , const Tier & tier
                            , size_t nProcessor ) {
    for( auto slot : tier[nProcessor]->data().slots() ) {
        // We use name from the slot to assure that processor will receive the
        // entry named exact as it was desired.
        vm._add_value_entry( slot->first
                           , ValueEntry{dataPtr, linkPtr} );
    }
}

void
Storage::free_values_map( ValuesMap & vm
                        , const Tier & tier
                        , size_t nProcessor ) {
}

// Worker
////////

Worker::Worker() {}

# if 1
void
Worker::run() {
    // Allocate storage
    Storage context( _dwRef );
    size_t tierCount = 0;
    for( auto & tier : tiers ) {
        // Bitmask reflecting one-to-one bits for processing
        Bitset toProcess( tier.size );
        toProcess.set();
        while( toProcess.any() ) {
            size_t nProcCurrent = tier.borrow_one();
            // Retrieve the data
            ValuesMap vm;
            context.build_values_map( vm, tier, nProcCurrent );
            // Here the actual processing goes
            tier[nProcCurrent].eval(vm);
            // Release the processor, drop "interest" bit
            tm.set_free(nProcCurrent);
            context.free_values_map( vm, tier, nProcCurrent );
            toProcess.clear( nProcCurrent );
        }
        assert( toProcess.none() );  // assure all done
    }
}
# endif

# if 0
// Testing fixture
/////////////////

// Represents a worker thread operating with tier monitor.
template<size_t NTasks> void
worker( TierMonitor<NTasks> & tm
      , std::chrono::milliseconds msecDelay
      , size_t nThread ) {
    std::vector<size_t> visitedOnes;

    std::bitset<NTasks> toProcess;
    for( size_t n = 0; n < NTasks; ++n ) { toProcess.set(n); }

    while( toProcess.any() ) {
        size_t one2Procs = tm.borrow_one( toProcess );
        std::cout << "#" << nThread
                  << "(" << toProcess << ") got " << one2Procs << std::endl;
        {   // Here the real job with one2Procs-th processor in tier has to be
            // performed.
            visitedOnes.push_back( one2Procs );
            std::this_thread::sleep_for(msecDelay);
        }
        tm.set_free(one2Procs);
        toProcess.reset(one2Procs);
        std::cout << "#" << nThread
                  << "(" << toProcess << ") done with " << one2Procs << std::endl;
    }

    assert( toProcess.none() );
}

int
main(int arc, char * argv[]) {
    TierMonitor<NProcessors> tm;
    using namespace std::chrono_literals;

    size_t nThreads = atoi(argv[1]);
    std::thread ** threads;
    threads = new std::thread * [nThreads];
    for( size_t n = 0; n < nThreads; ++n ) {
        size_t msecDelay = (rand()/double(RAND_MAX))*1000;
        threads[n] = new std::thread( worker<NProcessors>
                                    , std::ref(tm)
                                    , msecDelay*1ms
                                    , n );
    }
    for( size_t n = 0; n < nThreads; ++n ) {
        threads[n]->join();
    }
}
# endif

}  // namespace goo::dataflow
}  // namespace goo
