# include "goo_dataflow/framework.hpp"

struct gfw_link_pair_hash {
    typedef goo::dag::Node<goo::dataflow::iProcessor> * T1;
    typedef typename goo::dataflow::iProcessor::Ports::const_iterator T2;
    size_t operator()( const std::pair<T1, T2> & p ) const {
        return std::hash<T1>()(p.first)
             ^ std::hash<std::string>()(p.second->first)
             ;
    }
};

namespace goo {
namespace dataflow {

Framework::Framework() : _isCacheValid(false) {}

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

std::pair< typename iProcessor::Ports::const_iterator
         , typename iProcessor::Ports::const_iterator>
Framework::_assure_link_valid(
                     dag::Node<iProcessor> & nodeA, const std::string & aPortName
                   , dag::Node<iProcessor> & nodeB, const std::string & bPortName
                   ) {
    auto itPortA = nodeA.data().ports().find( aPortName )
       , itPortB = nodeB.data().ports().find( bPortName )
       ;
    if( nodeA.data().ports().end() == itPortA ) {
        emraise( noSuchKey, "Processor %p have no port named \"%s\"."
               , &(nodeA.data()), aPortName.c_str() );
    }
    if( nodeB.data().ports().end() == itPortB ) {
        emraise( noSuchKey, "Processor %p have no port named \"%s\"."
               , &(nodeB.data()), bPortName.c_str() );
    }
    if( *(itPortA->second.typeInfo) != *(itPortB->second.typeInfo) ) {
        emraise( badCast, "Type mismatch: while trying to link port \"%s\":%s"
                " of processor %p with port \"%s\":%s of processor %p."
                , itPortA->first.c_str(), itPortA->second.typeInfo->name(), &(nodeA.data())
                , itPortB->first.c_str(), itPortB->second.typeInfo->name(), &(nodeB.data())
                );
    }
    return std::make_pair(itPortA, itPortB);
}

size_t
Framework::precedes( iProcessor * a, const std::string & aPortName
                   , iProcessor * b, const std::string & bPortName ) {
    dag::Node<iProcessor> & nodeA = _get_node_by_proc_ptr( a )
                        , & nodeB = _get_node_by_proc_ptr( b );
    auto [itPortA, itPortB] = _assure_link_valid( nodeA, aPortName, nodeB, bPortName );
    nodeA.depends_on( nodeB );
    // TODO -- pool?
    Link * l = new Link{ _links.size(), nodeA, nodeB, itPortA, itPortB, 0 };
    _links.insert( l );
    _isCacheValid = false;
    return l->id;
}

void
Framework::_free_cache() const {
    for( auto tierPtr : _cache.tiers ) {
        delete tierPtr;
    }
    _cache.tiers.clear();
}

void
Framework::_recache() const {
    _free_cache();

    // Compute order of execution
    _cache.order = dag::dfs(_nodes);

    // To find out the values that has to be allocated, we have to consider all
    // connections between processors in a way similar to search in relational
    // table where following conditions steer the type of the value in storage:
    //  - links having the same destination port (on same processors) have to
    //  be considered as `join' value type;
    //  - links having the same source port (on same processors) have to
    //  be considered as `split' value type;
    //  - links with unique source and destination ports have to have their own
    //  unique values.
    std::unordered_map<std::pair< dag::Node<iProcessor>*
                                , typename iProcessor::Ports::const_iterator>
                      , size_t, gfw_link_pair_hash> joinValues
                                                  , splitValues
                                                  , standaloneValues
                                                  ;
# if 0
    for( auto lPtr : _links ) {
        auto jIt =       joinValues.find( std::make_pair(lPtr->nf, lPtr->fp) )
           , sIt =      splitValues.find( std::make_pair(lPtr->nf, lPtr->fp) )
           , vIt = standaloneValues.find( std::make_pair(lPtr->nf, lPtr->fp) )
           ;
        //dataOffset += linkPtr->from()->second.typeInfo->typeSize;
    }
# endif
    size_t dataOffset = 0;

    // Compute out the worker storage and tiers structures
    std::list<Tier *> tiers;
    for(auto tierNodes : _cache.order) {
        tiers.push_back(new Tier(tierNodes));
        size_t nodeNum = 0;
        for(auto dagNodePtr : tierNodes) {
            auto nodePtr = static_cast<dag::Node<iProcessor *>*>(dagNodePtr);
            iProcessor * p = nodePtr->data();
            for( auto portPtr : p->ports() ) {
                // Get links for this port!
                //dataSize += portPtr->second.typeSize;
                //slotPtr->name()
                //slotPtr->type()
            }
        }
    }
}

const Framework::Cache &
Framework::get_cache() const {
    if( !_isCacheValid ) {
        _recache();
    }
    return _cache;
}

}  // ::goo::dataflow
}  // goo

