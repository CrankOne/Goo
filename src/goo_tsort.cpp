# include <goo_tsort.tcc>

namespace goo {
namespace dag {

void
visit( DAGNode & n
     , std::vector< std::unordered_set<DAGNode*> > & l ) {
    if( n.has_temporary_mark() ) return;
    if( n.has_visited_mark() ) {
        emraise( badState, "Not a DAG: circular graph." );
    }
    n.mark_as_temporary();
    for( auto & d : n ) {
        if(! d->is_marked()) {
            visit( *d, l );
        }
        if( d->depth() >= n.depth() ) {
            n.set_depth( d->depth() + 1);
        }
    }
    n.clear_temporary();
    n.mark_as_visited();
    if( l.size() <= n.depth() ) {
        l.resize( n.depth() + 1 );
    }
    l[n.depth()].insert(&n);
}


std::vector<std::unordered_set<DAGNode*> >
dfs( std::unordered_set<DAGNode*> & s ) {
    std::vector<std::unordered_set<DAGNode*> > l;
    for( auto nPtr : s ) {
        if( !nPtr->is_marked() ) {
            visit( *nPtr, l );
        }
    }
    for( auto nPtr : s ) {
        assert( nPtr->is_marked() );
        nPtr->reset_dfs_descriptor();
    }
    return l;
}

}  // namespace dag
}  // namespace goo

