# pragma once

# include "goo_tsort.tcc"
# include "goo_dataflow/processor.hpp"
# include "goo_dataflow/tier.hpp"

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
protected:
    struct Link {
        dag::Node<iProcessor> & nf, & nt;
        typename iProcessor::Ports::const_iterator fp, tp;
    };
    /// Caches derived from _links & _nodes.
    struct Cache {
        std::unordered_map< const iProcessor *
                          , dag::Node<iProcessor> *> nodesByProcPtr;
        // Updated only during `_recache()' operation:
        /// Order of nodes processing.
        dag::Order order;
        /// Tiers storage.
        std::list<Tier *> tiers;
    };
    const Cache & get_cache() const;
private:
    std::unordered_set<dag::DAGNode*> _nodes;
    std::unordered_map<size_t, Link> _links;

    mutable bool _isCacheValid;
    mutable Cache _cache;
    void _free_cache() const;

    /// Returns Node by processor pointer. If processor has not been added
    /// before, allocates new node.
    dag::Node<iProcessor> & _get_node_by_proc_ptr( iProcessor * );
    /// Check type compatibilities of requested link ports and returns a pair
    /// of iterators for link construction.
    std::pair< typename iProcessor::Ports::const_iterator
             , typename iProcessor::Ports::const_iterator> _assure_link_valid(
                     dag::Node<iProcessor> & a, const std::string & aPortName
                   , dag::Node<iProcessor> & b, const std::string & bPortName
                   );
    static void _build_values_map( const std::unordered_map<size_t, Link> & );
public:
    Framework();

    /// Makes processor A to precede processor B with default (copying) link.
    size_t precedes( iProcessor * a, const std::string & aPortName
                   , iProcessor * b, const std::string & bPortName );

    // TODO: move under `protected':
    void _recache() const;

    friend class Storage;
};

}  // ::goo::dataflow
}  // goo
