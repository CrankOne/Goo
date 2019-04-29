# include "goo_dataflow/worker.hpp"

# include <vector>

namespace goo {
namespace dataflow {

Worker::Worker() {}

void
Worker::run( ) {
    size_t nBytesTLS = 0;
    // TODO: nBytesTLS += ...
    std::vector<char> data(nBytesTLS);
    // ...
}

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
