// TODO: delete

# if 0

# include <forward_list>
# include <unordered_set>
# include <unordered_map>
# include <set>
# include <iostream>  // XXX
# include <cstdlib>   // XXX

template<typename AssociciatedDataT>
class DependencyGraph {
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

        const Data * data() const { return _data; }
        bool is_terminal() const { return _deps.empty(); }
        const std::set<const Node *> & dependencies() const { return _deps; }

        size_t run_dfs_on_me( Order & ordered ) const {
            if( _status == opened ) {
                std::cerr << "Circular graph." << std::endl;
                abort();
            }
            _status = opened;
            if( is_terminal() ) {
                ordered.push_front( _data );
                _status = discovered;
                return 1;
            }
            //std::cout << *_data << std::endl;  // XXX
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
private:
    std::unordered_set<Node *> _nodes;
    std::unordered_map<const Data *, Node *> _dict;
public:
    DependencyGraph() {}
    DependencyGraph( const Data * bgn, size_t n ) {
        for( const Data * cd = bgn; cd != bgn + n; ++cd ) {
            auto nn = new Node(cd);
            _nodes.insert( nn );
            _dict[cd] = nn;
        }
    }
    ~DependencyGraph( ) {
        clear();
    }

    /// Inserts node with given data ptr.
    void insert( const Data * d ) {
        auto nn = new Node(d);
        _nodes.insert( nn );
        _dict[d] = nn;
    }

    /// Returns Node by specific data pointer.
    Node * operator()( const Data * dp ) {
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
    DependencyGraph revert() const {
        DependencyGraph rg;
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
    size_t dfs( Order & ordered ) const {
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

    // TODO: move it to more specific subclass -- it is required that data()
    // should support << operator.
    void dump_DOT_notation( std::ostream & os ) {
        os << "digraph Units {" << std::endl;
        for( auto it = _nodes.begin(); it != _nodes.end(); ++it ) {
            os << "    " << *(*it)->data() << ";" << std::endl;
        }
        for( auto it = _nodes.begin(); it != _nodes.end(); ++it ) {
            for( auto depIt = (*it)->dependencies().begin();
                      (*it)->dependencies().end() != depIt; ++depIt ) {
                os << "    " << *(*depIt)->data() << " -> " << *(*it)->data()
                   << ";" << std::endl;
            }
        }
        os << "}" << std::endl;
    }

    void chain_for_node( const Data * d, Order & order ) {
        auto r = this->revert();
        r(d)->run_dfs_on_me( order );
    }
};  // class DependencyGraph


int
main(int argc, char * argv[]) {

    std::string labels[] = {
        "A",    "B",    "C",
             "D",   "E",
        "F",    "G",    "H" };

    DependencyGraph<std::string> g;
    for( uint8_t i = 0; i < 8; ++i ) {
        g.insert( labels + i );
    }

    if(1) {  // set to zero for trivial case
        g(labels+5)->depends_from(g(labels+3));
        g(labels+6)->depends_from(g(labels+3)).depends_from(g(labels+4));
        g(labels+7)->depends_from(g(labels+3)).depends_from(g(labels+2));
        g(labels+3)->depends_from(g(labels+0)).depends_from(g(labels+1));
        g(labels+4)->depends_from(g(labels+0)).depends_from(g(labels+2)).depends_from(g(labels+7));
        // Add circular dependence for check:
        g(labels+3)->depends_from(g(labels+4));  // D from E
    }

    if(0) {
        g.dump_DOT_notation(std::cout);
    }

    if(1) {
        std::forward_list<const std::string *> order;
        size_t nchains = g.dfs( order );
        std::cout << "Order of " << nchains << " chains:" << std::endl;
        for( auto it = order.begin(); it != order.end(); ++it ) {
            std::cout << " " << **it << std::endl;
        }
    }

    // Solution for single node
    if(0) {  // Check validity of reverted graph
        DependencyGraph<std::string> rg = g.revert();
        rg.dump_DOT_notation(std::cout);
    }

    if(0) {
        std::forward_list<const std::string *> order;
        auto c = labels + 0;
        g.chain_for_node( c, order );
        std::cout << "Order for \"" << *c << "\" node:" << std::endl;
        for( auto it = order.begin(); it != order.end(); ++it ) {
            std::cout << " " << **it << std::endl;
        }
    }

    return 0;
}
# endif

