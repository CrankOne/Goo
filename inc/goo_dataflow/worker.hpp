# pragma once

# include <typeinfo>
# include <vector>
# include <unordered_map>
# include <cstdint>
# include <thread>

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
    /// Empty implementation by default.
    virtual inline void _notify( size_t nProc
                               , size_t nTier
                               , EventCode evType ) {}
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

/**@class ReentrantWorkers
 * @brief Manages a set of reentrant workers processing a framework.
 *
 * Performs dynamic scheduling across a set of threads & workers. Functionality
 * is close to the worker pool.
 *
 * Supposed to be used from main thread only.
 *
 * @TODO: run threads, actually
 * */
template<typename T>
class ReentrantWorkers {
private:
    Bitset _wrksBusyFlags;
    std::mutex _wrksBusyMtx;
    bool _processingRequired, _done;
    std::condition_variable _busyCV;

    T ** _workers;
    std::thread ** _threads;

    /// Running in thread, provides waiting/holding operations.
    void _thread_monitor( size_t n ) {
        std::unique_lock<std::mutex> lock(_wrksBusyMtx);
        while( ! _done ) {
            while( ! _processingRequired ) {
                _busyCV.wait( lock );
            }
            // Check `done' flag to support termination wake-up.
            if( _done ) break;
            _wrksBusyFlags.set( n );
            _workers[n].run();
        }
    }
public:
    ReentrantWorkers() : _wrksBusyFlags(std::thread::hardware_concurrency())
                       , _processingRequired(false)
                       , _done(false) {
    }

    ReentrantWorkers( int nWorkers ) : _wrksBusyFlags(nWorkers)
                                     , _threads(nullptr)
                                     , _processingRequired(false)
                                     , _done(false) {
        _wrksBusyFlags.reset();
        _workers = new T* [nWorkers];
        bzero( _workers, sizeof(T*)*nWorkers );
    }

    /// Starts execution of single worker from the pool (async). Only the
    /// vacant worker may be started. Returns `false' if there is no workers
    /// currently being vacant.
    bool dispatch_one( Framework & fw ) {
        std::unique_lock<std::mutex> lock(_wrksBusyFlags);
        if( _wrksBusyFlags.all() ) {
            // No vacant workers.
            return false;
        }
        // Some of the pending workers has to mark itself as busy.
        _processingRequired = true;
        _busyCV.notify_one();
        _processingRequired = false;
        return true;
    }

    /// Launches all the workers currently being available. Returns `false' if
    /// no workers had been activated.
    bool dispatch( Framework & fw ) {
        std::unique_lock<std::mutex> lock(_wrksBusyMtx);
        if( _wrksBusyFlags.all() ) {
            // No vacant workers.
            return false;
        }
        // Some of the pending workers has to mark itself as busy.
        _processingRequired = true;
        _busyCV.notify_all();
        _processingRequired = false;
        return true;
    }

    /// Blocks execution of caller untill all the workers is done.
    bool wait() {
        std::unique_lock<std::mutex> lock(_wrksBusyMtx);
        if( _wrksBusyFlags.all() ) {
            // No vacant workers.
            return false;
        }
        // Some of the pending workers has to mark itself as busy.
        _done = true;
        _busyCV.notify_all();
        _done = false;
        assert( _wrksBusyFlags.none() );
        return true;
    }
};

}  // namespace goo::dataflow
}  // namespace goo

