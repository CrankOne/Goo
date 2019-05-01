# include "goo_dataflow/framework.hpp"

namespace goo {
namespace dataflow {

dag::Node<iProcessor> &
Framework::_get_node_by_proc_ptr( iProcessor * p ) {
    auto it = _cache.nodesByProcPtr.find(p);
    if( _cache.nodesByProcPtr.end() != it ) return *(it->second);
    auto ir = _cache.nodesByProcPtr.emplace( p, new dag::Node<iProcessor>(*p) );
    if( !ir.second ) {
        emraise( badState, "Unable to emplace new processor entry." );
    }
    it = ir.first;
    _nodes.insert( it->second );
    return *(it->second);
}

Link &
Framework::precedes( iProcessor * a, iProcessor * b ) {
    dag::Node<iProcessor> & nodeA = _get_node_by_proc_ptr( a )
                        , & nodeB = _get_node_by_proc_ptr( b );
    nodeA.depends_on( nodeB );
    _links.push_back( new Link{ nodeB, nodeA } );
    _cache.linksByFrom.emplace( a, _links.back() );
    _cache.linksByTo.emplace( b, _links.back() );
    return *(_links.back());
}

const Framework::Cache &
Framework::get_cache() const {
    return _cache;
}

}  // ::goo::dataflow
}  // goo

