# ifndef H_TOPOLOGICAL_SORT_H
# define H_TOPOLOGICAL_SORT_H

# include <forward_list>
# include <unordered_set>
# include <unordered_map>
# include <ostream>
# include <set>

# include "goo_exception.hpp"

namespace goo {

template<typename AssociciatedDataT>
class DAG {
public:
    typedef AssociciatedDataT Data;
    typedef std::forward_list<const Data *> Order;

    /// Auxilliary internal node class.
    class Node {
    public:
        mutable enum DfsStatus {
            undiscovered,
            opened,
            discovered,
        } _status;
    private:
        std::set<const Node *>   _deps;
        const Data             * _data;
    public:
        Node( const Data * d ) : _status(undiscovered), _data(d) {}
        Node( Node * nodes,
              size_t n ) : _status(undiscovered),
                           _deps(nodes, nodes+n) {}
        Node( Node * nodes, size_t n,
              const Data * d ) : _status(undiscovered),
                                 _deps(nodes, nodes+n),
                                 _data(d) {}

        Node & depends_from( const Node * nd ) {
            _deps.insert(nd);
            return *this; }
        virtual ~Node(){}

        const Data * data() const { return _data; }
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
    std::unordered_set<Node *> _nodes;
    std::unordered_map<const Data *, Node *> _dict;
public:
    DAG() {}
    DAG( const Data * bgn, size_t n ) {
        for( const Data * cd = bgn; cd != bgn + n; ++cd ) {
            auto nn = new Node(cd);
            _nodes.insert( nn );
            _dict[cd] = nn;
        }
    }
    virtual ~DAG( ) {
        clear();
    }

    /// Inserts node with given data ptr.
    virtual void insert( const Data * d ) {
        auto nn = new Node(d);
        _nodes.insert( nn );
        _dict[d] = nn;
    }

    /// Returns Node by specific data pointer.
    virtual Node * operator()( const Data * dp ) {
        return _dict[dp];
    }

    /// Deletes nodes.
    void clear() {
        for( auto it = _nodes.begin();
             _nodes.end() != it; ++it ) {
            delete (*it);
        }
        _nodes.clear();
        _dict.clear();
    }

    /// Build a graph with reversed dependencies.
    DAG revert() const {
        DAG rg;
        // Place copy of nodes withoud pes.
        for( auto it = _nodes.begin();
             _nodes.end() != it;
             ++it ) {
            rg.insert( (*it)->data() );
        }
        // Establish reverted connections.
        for( auto it = _nodes.begin();
             _nodes.end() != it;
             ++it ) {
            Node * cn = rg( (*it)->data() );
            const Node * on = _dict.find((*it)->data())->second;
            for( auto depIt  = on->dependencies().begin();
                      on->dependencies().end() != depIt; ++depIt) {
                rg((*depIt)->data())->depends_from( rg(cn->data()) );
            }
        }
        return rg;
    }

    /// Provides a depth-first search.
    virtual size_t dfs( Order & ordered ) const {
        size_t nChains = 0,
               N = 0;  // overall processed
        while( N != _nodes.size() ) {
            size_t nprocessed = 0;
            for( auto it = _nodes.begin(); it != _nodes.end(); ++it ) {
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
    }
};  // class DAG

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

        Node & depends_from( std::string & depLbl ) {
            Parent::Node::insert( &(_owner(depLbl)) );
            return *this; }
        const std::string & label() const { return _label; }
    };
private:
    std::unordered_map<std::string, Node *> _byLabels;
public:

    Node & operator()(const std::string & l ) {
        return *_byLabels[l];
    }

    /// Deletes nodes.
    virtual void clear() {
        Parent::clear();
        _byLabels.clear();
    }

    /// Inserts node with given data ptr.
    virtual void insert( const std::string & label, const Data * d ) {
        auto nn = new Node(label, d, *this);
        Parent::_nodes.insert( nn );
        Parent::_dict[d] = nn;
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

    virtual void chain_for_node( const std::string & label, Order & order ) {
        Parent::chain_for_node( _byLabels[label]->data(), order );
    }

    const std::unordered_map<std::string, Node *> & index() const {
        return _byLabels;
    }
};

}  // namespace goo

# endif  // H_TOPOLOGICAL_SORT_H

