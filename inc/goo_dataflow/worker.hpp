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
    struct LogEntry {
        static constexpr uint8_t execStarted = 0x1;
        static constexpr uint8_t execEnded = 0x2;

        uint8_t type;
        size_t nProc, nTier;
        std::clock_t time;
    };
protected:
    Framework & _fwRef;
    mutable std::mutex _logSyncM;
    std::vector <LogEntry> _log;
public:
    Worker( Framework & fr ) : _fwRef(fr) {}
    /// Must be passed to a std::thread.
    void run();
    /// Log. TODO: sync access.
    const std::vector<LogEntry> log() const {
        std::unique_lock<std::mutex> l(_logSyncM);
        return _log;
    }
};

}  // namespace goo::dataflow
}  // namespace goo

