# include "goo_tsort.tcc"
# include "goo_dataflow/processor.hpp"
# include "goo_dataflow/tier.hpp"

# pragma once

namespace goo {
namespace dataflow {

/**@brief Represents a complex data processing algorithm.
 * @class Framework
 *
 * The framework describes how the set of routines are connected together in
 * a direct acyclic graph (DAG).
 *
 * The framework does not maintain the lifetime of processor instances. Its
 * primary function to gurantee simultaneous update procedures of various
 * caches used during parallel treatment using Worker class instances.
 *
 * TODO: Might be ran using input operator (<<), or, if all the data generated
 * directly in DAG.
 * */
class Framework {
private:
    std::unordered_set<dag::DAGNode*> _nodes;
    std::list<Link *> _links;
    /// Caches derived from _links & _nodes.
    struct {
        std::unordered_map< const iProcessor *
                          , dag::Node<iProcessor> *> nodesByProcPtr;
        std::unordered_map< const iProcessor *
                          , Link * > linksByFrom
                                   , linksByTo;
    } _cache;
protected:
    /// Assembles tiers and TLS caches.
    void _build_context();
    /// Returns Node by processor pointer. If processor has not been added
    /// before, allocates new node.
    dag::Node<iProcessor> & _get_node_by_proc_ptr( iProcessor * );
public:
    /// Makes processor A to precede processor B.
    Link & precedes( iProcessor * a, iProcessor * b );  // TODO: rename to `precedes'
};

}  // ::goo::dataflow
}  // goo
