# pragma once

# include <typeinfo>
# include <vector>
# include <unordered_map>
# include <cstdint>

# include "goo_dataflow/processor.hpp"
# include "goo_dataflow/tier.hpp"
# include "goo_dataflow/framework.hpp"

namespace goo {
namespace dataflow {

/**@brief Thread local storage for a worker.
 * @class Storage
 *
 * Implements a dynamic buffer providing thread-local storage for the data used
 * by links in DAG.
 *
 * @TODO custom allocation/deletion of data, lifecycle hooks, std::allocator
 * support
 * */
class Storage : public std::vector<uint8_t> {
private:
    std::vector<ValuesMap> * _vms;
protected:
    Storage( const Framework::Cache & );
    ~Storage();
    /// Builds variables map for certain processor in certain tier.
    ValuesMap & values_map_for( size_t tierNo, size_t processorNo );

    friend class Worker;
};

/**@class Worker
 * @brief Defines thread-local context to be utilized during graph traversal.
 * */
class Worker {
public:
    enum EventCode : uint8_t {
        execStarted,        // on processing start
        execOk,             // on EvalStatus::ok
        execTerm,           // in case of termination by external reason
        execErrException,   // on exception
        execRuntimeError,   // on EvalStatus::error
        execSkip,           // on EvalStatus::skip
        execDone,           // on EvalStatus::done
        execBadRC,          // otherwise
    };
protected:
    /// Reference to the framework instance to be executed.
    Framework & _fwRef;
    /// TODO: XXX
    virtual inline void _notify( size_t nProc, size_t nTier, EventCode evType ) {
        //if( evType & _stopFlag ) {
        //    // In this case, all the concurrent threads has to be notified that
        //}
    }
    /// Ptr to exception caught, if any.
    std::exception_ptr _excPtr;
public:
    Worker( Framework & fr ) : _fwRef(fr), _excPtr(nullptr) {}
    /// Must be passed to a std::thread.
    void run();
    /// Returns current exception pointer in case of malfunction.
    std::exception_ptr exception_ptr() const { return _excPtr; }
};

/**@class JournalingWorker
 * @brief An implementation of Worker class with journaling capabilities.
 *
 * Performs simple logging during execution. Useful for debugging.
 * */
class JournaledWorker : public Worker {
public:
    struct LogEntry {
        Worker::EventCode type;
        size_t nProc, nTier;
        std::clock_t time;
    };
protected:
    /// Synchronizes access to internal logging journal.
    mutable std::mutex _logSyncM;
    /// Logging journal.
    std::vector <LogEntry> _log;
    /// Adss `processing started' event to the journal.
    virtual inline void _notify( size_t nProc, size_t nTier, Worker::EventCode event ) override {
        std::unique_lock<std::mutex> l(_logSyncM);
        _log.push_back( LogEntry{ event
                                , nProc, nTier
                                , std::clock() } );
    }
public:
    JournaledWorker( Framework & fr ) : Worker(fr) {}
    /// returns copy of journal for external usage.
    const std::vector<LogEntry> log() const {
        std::unique_lock<std::mutex> l(_logSyncM);
        return _log;
    }
};

}  // namespace goo::dataflow
}  // namespace goo

