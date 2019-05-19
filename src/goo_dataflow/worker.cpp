# include "goo_dataflow/worker.hpp"
# include "goo_exception.hpp"

namespace goo {
namespace dataflow {

Storage::Storage( const Framework::Cache & fwc ) {
    std::vector<uint8_t>::resize(fwc.dataSize);
    _vms = new std::vector<ValuesMap> [fwc.tiers.size()];
    // Allocate storage according to structure provided by framework's cache
    size_t nTier = 0;
    // * in each tier
    assert(!fwc.tiers.empty());
    for( auto tierPtr : fwc.tiers ) {
        assert(!tierPtr->empty());
        _vms[nTier].resize(tierPtr->size());
        size_t nProc = 0;
        // * for each processor
        for( auto nodePtr : *tierPtr ) {
            ValuesMap & vm = _vms[nTier][nProc];
            // * build the values map entries to hold the data pointer
            for( auto portIt = nodePtr->data().ports().cbegin()
               ; nodePtr->data().ports().cend() != portIt
               ; ++portIt) {
                size_t linkID;
                Framework::Cache::BoundPort_t bp( nodePtr, portIt );
                if( portIt->second.is_input() ) {
                    auto linkIt = fwc.byDstLinked.find( bp );
                    if( fwc.byDstLinked.end() == linkIt) {
                        emraise( badState, "Input port %p:\"%s\" does not"
                                " refer to any link in DAG."
                                , nodePtr, portIt->first.c_str() );
                    }
                    linkID = linkIt->second;
                } else if( portIt->second.is_output() ) {
                    auto linkIt = fwc.bySrcLinked.find( bp );
                    if( fwc.bySrcLinked.end() == linkIt) {
                        // TODO: turn in warning?
                        emraise( badState, "Output port %p:\"%s\" does not"
                                " refer to any link in DAG."
                                , nodePtr, portIt->first.c_str() );
                    }
                    linkID = linkIt->second;
                } else {
                    emraise( badState, "Port connection %p:\"%s\" has no I/O"
                            " markings.", nodePtr, portIt->first.c_str() );
                }
                auto layoutIt = fwc.layoutMap.find( linkID );
                if( fwc.layoutMap.end() == layoutIt ) {
                    emraise( badState, "Framework data layout map does not"
                            " provide offset value for link %zu"
                            " (port %p:\"%s\")."
                            , linkID, nodePtr, portIt->first.c_str() );
                }
                vm.add_value_entry( portIt->first
                                  , ValueEntry(this->data() + layoutIt->second) );
            }
            ++nProc;
        }
        ++nTier;
    }
}

Storage::~Storage() {
    delete [] _vms;
}

ValuesMap &
Storage::values_map_for( size_t tierNo
                       , size_t processorNo ) {
    return _vms[tierNo][processorNo];
}

// Worker
////////

//Worker::Worker() {}

void
Worker::run() {
    // Allocate storage
    Storage context( _fwRef.get_cache() );
    size_t tierCount = 0;
    for( auto tierPtr : _fwRef.get_cache().tiers ) {
        auto & tier = *tierPtr;
        // Bitmask reflecting one-to-one bits for processing
        Bitset toProcess( tier.size() );
        toProcess.set();
        while( toProcess.any() ) {
            dag::Node<iProcessor> * nPtr;
            size_t nProcCurrent = tier.borrow_one( toProcess, nPtr );
            {
                std::unique_lock<std::mutex> l(_logSyncM);
                _log.push_back( LogEntry{ LogEntry::execStarted
                                        , nProcCurrent, tierCount
                                        , std::clock() } );
            }
            // Here the actual processing goes
            //try {
                nPtr->data().eval(
                        context.values_map_for( tierCount, nProcCurrent )
                    );
            //} catch( std::exception & e ) {
            //  ... TODO
            //}
            // Release the processor, drop "interest" bit
            tier.set_free(nProcCurrent);
            toProcess.reset( nProcCurrent );
            {
                std::unique_lock<std::mutex> l(_logSyncM);
                _log.push_back( LogEntry{ LogEntry::execEnded
                                        , nProcCurrent, tierCount
                                        , std::clock() } );
            }
        }
        assert( toProcess.none() );  // assure all done
        ++tierCount;
    }
}

}  // namespace goo::dataflow
}  // namespace goo
