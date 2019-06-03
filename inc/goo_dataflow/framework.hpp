# pragma once

# include <mutex>

# include "goo_tsort.tcc"
# include "goo_dataflow/processor.hpp"
# include "goo_dataflow/tier.hpp"

namespace goo {
namespace dataflow {

class Worker;
class Storage;

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
public:
    /// Execution node type: all the nodes in the Framework are of this type.
    typedef dag::Node<iProcessor> ExecNode;
protected:
    struct Link {
        ExecNode & nf, & nt;
        typename iProcessor::Ports::const_iterator fp, tp;
    };
    /// Cache derived from links & nodes. Must not be changed while worker
    /// thread(s) running.
    struct Cache {
        /// A special indexing key: ports are uniquely addressed by their node
        /// AND port declaration iterator.
        typedef std::pair< dag::Node<iProcessor> *
                         , typename iProcessor::Ports::const_iterator> BoundPort_t;
        /// Special less operator for (multi-)maps.
        struct BoundLinkLess {
            bool operator()( const BoundPort_t &, const BoundPort_t & ) const;
        };
        /// Order of nodes processing.
        dag::Order order;
        /// Processing tiers storage.
        std::list<Tier *> tiers;
        /// Lists link IDs by their connected ports.
        std::multimap<BoundPort_t, size_t, BoundLinkLess> bySrcLinked
                                                        , byDstLinked;
        // Index keeps linkID vs offset.
        std::unordered_map<size_t, size_t> layoutMap;
        // Overall data size to be allocated.
        size_t dataSize;
    };
    const Cache & get_cache() const;
private:
    /// All nodes created are stored in this set.
    std::unordered_set<dag::DAGNode*> _nodes;
    /// By-name index of nodes within the framework. Note, that it does not
    /// necessarily contain all the nodes within a framework.
    std::unordered_map<std::string, ExecNode *> _nodesByName;
    /// Reverse index, referring to node names by their pointers used in some
    /// verbose utils.
    std::unordered_map<ExecNode *, std::string> _namesByNodes;
    /// All connections b/w framework nodes are indexed here. This is only an
    /// accompanying information to what the Goo's DAG implementation provides.
    std::unordered_map<size_t, Link> _links;

    /// Controls, whether the cache have to be re-computed.
    mutable bool _isCacheValid;
    /// Cache built on a framework: indexes, storage layout, etc.
    mutable Cache _cache;
    /// Mutex protecting cache.
    mutable std::recursive_mutex _cacheMtx;
    
    /// When set to true, every worker has to terminate traversal through DAG.
    bool _terminateFlag;
    /// Guards _errorFlag
    mutable std::mutex _terminateFlagMtx;

    /// Marks cache as invalid.
    void _invalidate_cache() const { _isCacheValid = false; }
    /// Performs cache cleanup.
    void _free_cache() const;
    /// Re-caches various indexes stored in _cache member.
    void _recache() const;

    /// Check type compatibilities of requested link ports and returns a pair
    /// of iterators for link construction.
    std::pair< typename iProcessor::Ports::const_iterator
             , typename iProcessor::Ports::const_iterator> _assure_link_valid(
                     ExecNode & a, const std::string & aPortName
                   , ExecNode & b, const std::string & bPortName
                   );
    static void _build_values_map( const std::unordered_map<size_t, Link> & );
protected:
    /// Sets the error flag.
    void _set_terminate_flag();
public:
    /// Default ctr -- empty framework.
    Framework();

    /// Erases all data.
    ~Framework();

    /// Emplaces processor in Framework producing a new node. Note: one can
    /// impose one processor instance few times (being aware of control over
    /// its in this case). Note, that for nameless nodes the only way to
    /// retreive it from framework is by ptr.
    ExecNode * impose( iProcessor & );

    /// Emplaces processor in Framework producing a new named node. Note: one
    /// can impose one processor instance few times (being aware of control
    /// over its in this case)
    ExecNode * impose( const std::string &, iProcessor & );

    /// Returns node pointer by name.
    const std::unordered_map<std::string, ExecNode *> & named_nodes( ) {
        return _nodesByName; }

    ExecNode * get_processor_by_name(const std::string & name);
    ExecNode * operator[](const std::string & name);

    /// Makes processor A to precede processor B with default (copying) link.
    size_t precedes( ExecNode * a, const std::string & aPortName
                   , ExecNode * b, const std::string & bPortName );

    /// Makes processor A to precede processor B with default (copying) link.
    size_t precedes( const std::string & a, const std::string & aPortName
                   , const std::string & b, const std::string & bPortName );

    /// Prints the DAG information. Needs a valid cache.
    void generate_dot_graph( std::ostream & ) const;

    bool terminate_flag_is_set() const {
        std::unique_lock<std::mutex> l( _terminateFlagMtx );
        return _terminateFlag;
    }

    friend class Storage;
    friend class Worker;
};

}  // ::goo::dataflow
}  // goo
