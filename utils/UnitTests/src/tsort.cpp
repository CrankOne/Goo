# include <forward_list>
# include <unordered_set>
# include <set>
# include <iostream>  // XXX
# include <cstdlib>   // XXX

template<typename AssociciatedDataT>
class DependencyGraph {
public:
    typedef AssociciatedDataT Data;
    class Node {
    public:
        mutable enum DfsStatus {
            undiscovered,
            opened,
            discovered,
        } _status;
    private:
        std::set<const Node *>   _deps;
        const Data                       * _data;
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

        size_t run_dfs_on_me( std::forward_list<const Data *> & ordered ) const {
            if( is_terminal() ) {
                ordered.push_front( _data );
                _status = discovered;
                return 1;
            }
            //std::cout << *_data << std::endl;  // XXX
            _status = opened;
            size_t nprocessed = 0;
            for( auto it = _deps.begin(); it != _deps.end(); ++it ) {
                if( undiscovered == (*it)->_status ) {
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
    std::unordered_set<const Node *> _nodes;
public:
    DependencyGraph() {}
    DependencyGraph( const Node * bgn, size_t n ) {
        for( const Node * cn = bgn; cn != bgn + n; ++cn ) {
            _nodes.insert( cn );
        }
    }

    /// Provides a depth-first search.
    size_t dfs( std::forward_list<const Data *> & ordered ) const {
        size_t nChains = 0,
               N = 0;  // overall processed
        while( N != _nodes.size() ) {
            for( auto it = _nodes.begin(); it != _nodes.end(); ++it ) {
                if( Node::undiscovered != (*it)->_status ) {
                    continue;  // pass visited node.
                }
                // start on terminal undiscovered node:
                size_t nprocessed = (*it)->run_dfs_on_me( ordered );
                N += nprocessed;
                ++nChains;
            }
            //if( !nprocessed && !(*it)->is_terminal() ) {
            //    std::cerr << "Circular deps found." << std::endl;  // XXX
            //    //for( auto it = ordered.begin(); it != ordered.end(); ++it ) {
            //    //    std::cout << " " << **it << std::endl;
            //    //}
            //    abort();  // XXX
            //}
        }
        if( !nChains ) {
            std::cerr << "Empty or circular graph." << std::endl;  // XXX
            abort();  // XXX
        }
        return nChains;
    }
};  // class DependencyGraph


int
main(int argc, char * argv[]) {
    typedef DependencyGraph<std::string> Graph;

    std::string labels[] = {
        "7",    "5",    "3",
             "11",   "8",
        "2",    "9",    "10" };
    Graph::Node nodes[] = {
        {labels  }, {labels+1}, {labels+2},
             {labels+3}, {labels+4},
        {labels+5}, {labels+6}, {labels+7}
    };

    if(1) {  // set to zero for trivial case
        (nodes+5)->depends_from(nodes+3);
        (nodes+6)->depends_from(nodes+3).depends_from(nodes+4);
        (nodes+7)->depends_from(nodes+3).depends_from(nodes+2);
        (nodes+3)->depends_from(nodes+0).depends_from(nodes+1);
        (nodes+4)->depends_from(nodes+0).depends_from(nodes+2);
    }

    Graph g(nodes, 8);

    std::forward_list<const std::string *> order;

    size_t nchains = g.dfs( order );

    std::cout << "Order of " << nchains << " chains:" << std::endl;
    for( auto it = order.begin(); it != order.end(); ++it ) {
        std::cout << " " << **it << std::endl;
    }

    return 0;
}

