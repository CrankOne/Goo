# pragma once

namespace goo {
namespace dataflow {

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

