# pragma once

# include <typeinfo>
# include <vector>
# include <unordered_map>
# include <cstdint>

# include "goo_dataflow/processor.hpp"
# include "goo_dataflow/tier.hpp"

namespace goo {
namespace dataflow {

/**@brief Thread local storage for a worker.
 * @class TLS
 *
 * ...
 * */
class Storage : public std::vector<uint8_t> {
protected:
    Storage();
    /// Builds variables map for certain processor in certain tier.
    void build_variables_map( ValuesMap & vm
                            , const Tier & tier
                            , size_t nProcessor );
    /// Called to free variables map for certain processor in certain tier.
    void free_values_map( ValuesMap & vm
                        , const Tier & tier
                        , size_t nProcessor );
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

