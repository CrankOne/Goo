# ifndef H_TOPOLOGICAL_SORT_H
# define H_TOPOLOGICAL_SORT_H

# include <forward_list>
# include <unordered_set>
# include <unordered_map>
# include <ostream>
# include <set>

# include "goo_exception.hpp"
# include "goo_iterable.tcc"

namespace goo {

/**@class DAG
 * @brief Directed Acyclic Graph
 *
 * This template implements directed acyclic graph and some
 * routines utilizing it.
 * For details, see: https://en.wikipedia.org/wiki/Directed_acyclic_graph
 *
 * This DAG implementation is not so convinient as its labeled descendant,
 * but meets most of the strighforward needs.
 * (see LabeledDAG template)
 * */
template<typename AssociciatedDataT>
class DAG {
public:
    /// Defined data type template parameter.
    typedef AssociciatedDataT Data;

    /// Aux type representing particular order of graph traversal.
    typedef std::forward_list<const Data *> Order;

    /// Auxilliary internal node class.
    class Node {
    public:
        /// DFS search markers (for internal use).
        mutable enum DfsStatus {
            undiscovered,
            opened,
            discovered,
        } _status;
    private:
        std::set<const Node *>   _deps;
        /// Data constness should be guaranteed along all the DAG routines.
        const Data             * _data;
    public:
        /// Default node constructor from existing data pointer.
        Node( const Data * d ) : _status(undiscovered), _data(d) {}

        /// Ctr for dereferred data association.
        Node() : _status(undiscovered), _data(nullptr) {}

        /// Sets the relation 'node nd depends on this node'.
        Node & dependance_of( const Node * nd ) {
            _deps.insert(nd);
            return *this; }

        /// Sets the relation 'this node depends on nd'.
        const Node & depends_on( Node * nd ) const {
            nd->dependance_of( this );
            return *this; }

        /// dtr: no heap operations.
        virtual ~Node(){}

        /// Returns data pointer.
        const Data * data_ptr() const { return _data; }
        
        bool is_terminal() const { return _deps.empty(); }
        const std::set<const Node *> & dependencies() const { return _deps; }

        size_t run_dfs_on_me( Order & ordered ) const {
            if( _status == opened ) {
                emraise( badState, "Circular graph." );
            }
            _status = opened;
            if( is_terminal() ) {
                ordered.push_front( _data );
                _status = discovered;
                return 1;
            }
            size_t nprocessed = 0;
            for( auto it = _deps.begin(); it != _deps.end(); ++it ) {
                if( discovered != (*it)->_status ) {
                    nprocessed += (*it)->run_dfs_on_me( ordered );
                }
            }
            ordered.push_front( _data );
            _status = discovered;
            ++nprocessed;
            return nprocessed;
        }
    };  // class Node
protected:
    std::unordered_set<Node *> * _nodesPtr;
    std::unordered_map<const Data *, Node *> * _dictPtr,
                                             & _dict;
public:
    DAG() : _nodesPtr( new std::unordered_set<Node *>() ),
            _dictPtr(  new std::unordered_map<const Data *, Node *> ),
            _dict(*_dictPtr) {}
    DAG( const Data * bgn, size_t n ) :
            _nodesPtr( new std::unordered_set<Node *>() ),
            _dictPtr(  new std::unordered_map<const Data *, Node *> ),
            _dict(*_dictPtr) {
        for( const Data * cd = bgn; cd != bgn + n; ++cd ) {
            auto nn = new Node(cd);
            _nodesPtr->insert_data( nn );
            _dict[cd] = nn;
        }
    }
    //DAG( const DAG & orig );
    virtual ~DAG( ) {
        clear();
        delete _nodesPtr;
        delete _dictPtr;
    }

    /// Inserts node with given data ptr.
    virtual Node * insert_data( const Data * d ) {
        auto nn = new Node(d);
        _nodesPtr->insert( nn );
        _dict[d] = nn;
        return nn;
    }

    /// Returns Node by specific data pointer.
    virtual Node * operator()( const Data * dp ) {
        return _dict[dp];
    }

    /// Deletes nodes.
    void clear() {
        for( auto it = _nodesPtr->begin();
             _nodesPtr->end() != it; ++it ) {
            delete (*it);
        }
        _nodesPtr->clear();
        _dict.clear();
    }

    /// Build a graph with reversed dependencies.
    DAG revert() const {
        DAG rg;
        // Place copy of nodes withoud pes.
        for( auto it = _nodesPtr->begin();
             _nodesPtr->end() != it;
             ++it ) {
            rg.insert_data( (*it)->data_ptr() );
        }
        // Establish reverted connections.
        for( auto it = _nodesPtr->begin();
             _nodesPtr->end() != it;
             ++it ) {
            Node * cn = rg( (*it)->data_ptr() );
            const Node * on = _dict.find((*it)->data_ptr())->second;
            for( auto depIt  = on->dependencies().begin();
                      on->dependencies().end() != depIt; ++depIt) {
                rg((*depIt)->data_ptr())->dependance_of( rg(cn->data_ptr()) );
            }
        }
        return rg;
    }

    /// Provides a depth-first search.
    virtual size_t dfs( Order & ordered ) const {
        size_t nChains = 0,
               N = 0;  // overall processed
        while( N != _nodesPtr->size() ) {
            size_t nprocessed = 0;
            for( auto it = _nodesPtr->begin(); it != _nodesPtr->end(); ++it ) {
                if( Node::discovered == (*it)->_status ) {
                    continue;  // pass visited or open node.
                }
                // start on terminal undiscovered node:
                nprocessed = (*it)->run_dfs_on_me( ordered );
                N += nprocessed;
                ++nChains;
            }
        }
        return nChains;
    }

    virtual void chain_for_node( const Data * d, Order & order ) {
        auto r = this->revert();
        r(d)->run_dfs_on_me( order );
        // as the order obtaining in reverse
        order.reverse();
    }
};  // class DAG

/**@class LabeledDAG
 * @brief Directed Acyclic graph with labeled nodes and associated data.
 *
 * This class insertion and depth-searching routines.
 *
 * Note, that our DAG conception implies only pointer management (no
 * data manipulation routines provided).
 * */
template<typename AssociciatedDataT>
class LabeledDAG : protected DAG<AssociciatedDataT> {
public:
    typedef AssociciatedDataT Data;
    typedef DAG<Data> Parent;
    typedef typename Parent::Order Order;
    typedef LabeledDAG<Data> Self;
protected:
    class Node : public Parent::Node {
    private:
        std::string _label;
        Self & _owner;
    public:
        Node( std::string label,
              const Data * d,
              Self & set ) : Parent::Node(d),
                             _label(label),
                             _owner(set) {}

        Node & dependance_of( const std::string & depLbl ) {
            Parent::Node::dependance_of( &(_owner(depLbl)) );
            return *this; }
        Node & dependance_of( const Node * depNPtr ) {
            Parent::Node::dependance_of( depNPtr );
            return *this; }
        const std::string & label() const { return _label; }
    };
private:
    std::unordered_map<std::string, Node *> _byLabels;
public:

    LabeledDAG() : Parent() {}

    Node & get_node(const std::string & l ) {
        return *_byLabels[l];
    }

    Node & operator()(const std::string & l ) {
        return get_node(l);
    }

    const Node & get_node(const std::string & l ) const {
        return *_byLabels[l];
    }

    const Node & operator()(const std::string & l ) const {
        return get_node(l);
    }

    /// Deletes nodes.
    virtual void clear() {
        Parent::clear();
        _byLabels.clear();
    }

    /// Inserts node with given data ptr.
    virtual void insert( const std::string & label, const Data * d ) {
        auto nn = new Node(label, d, *this);
        assert( Parent::_dictPtr );
        Parent::_dict[d] = nn;
        Parent::_nodesPtr->insert( nn );
        _byLabels[label] = nn;
    }

    /// Renders dependency graph in DOT notation. Useful for debug.
    void dump_DOT_notated_graph( std::ostream & os ) {
        os << "digraph Units {" << std::endl;
        for( auto it = _byLabels.begin(); it != _byLabels.end(); ++it ) {
            os << "    " << it->first << ";" << std::endl;
        }
        for( auto it = _byLabels.begin(); it != _byLabels.end(); ++it ) {
            for( auto depIt = (*it).second->dependencies().begin();
                      (*it).second->dependencies().end() != depIt; ++depIt ) {
                os << "    " << static_cast<Node*>(it->second)->label() << " -> "
                             << static_cast<Node*>(it->second)->label()
                   << ";" << std::endl;
            }
        }
        os << "}" << std::endl;
    }

    /// Provides a depth-first search.
    virtual size_t dfs( Order & ordered ) const {
        return Parent::dfs(ordered);
    }

    /// Obtain chain for node pointed out by particular label.
    virtual void chain_for_node_by_label( const std::string & label, Order & order ) {
        Parent::chain_for_node( _byLabels[label]->data_ptr(), order );
    }

    /// Returns list of all nodes.
    const std::unordered_map<std::string, Node *> & index() const {
        return _byLabels;
    }
};

}  // namespace goo

# endif  // H_TOPOLOGICAL_SORT_H

