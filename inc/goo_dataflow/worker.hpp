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
 * @class TLS
 *
 * ...
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

/** A worker constitues thread-local context to be utilized during graph
 * traversal.
 * */
class Worker {
protected:
    Framework & _fwRef;
public:
    Worker( Framework & fr ) : _fwRef(fr) {}
    /// Must be passed to a std::thread.
    void run();
};

}  // namespace goo::dataflow
}  // namespace goo

