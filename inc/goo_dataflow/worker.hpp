# pragma once

# include <typeinfo>
# include <vector>
# include <unordered_map>
# include <cstdint>

namespace goo {
namespace dataflow {

struct DataBlock {
    size_t offset;
    std::type_info * typeInfo;
};

/**@brief Thread local storage for a worker.
 * @class TLS
 *
 * ...
 * */
class TLS : private std::vector<uint8_t>
          , private std::unordered_map<std::string, DataBlock>  {
public:
    TLS();
    template<typename T> void new_value();
};

/** A worker constitues thread-local context to be utilized during graph
 * traversal.
 * */
class Worker {
public:
    Worker();
    /// Must be passed to a std::thread.
    void run();
};

}  // namespace goo::dataflow
}  // namespace goo

