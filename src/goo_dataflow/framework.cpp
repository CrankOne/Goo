# include "goo_dataflow/framework.hpp"

namespace goo {
namespace dataflow {

namespace aux {
// This hashing function probably does not provide hash being unique enough to
// rely on it as collision-free, though providing us some boost with
// unique-finding routines need in Framework::recache() method. Moreover,
// the resulting hash value depends on pointer.
struct gfw_link_pair_hash {
    typedef goo::dag::Node<goo::dataflow::iProcessor> * T1;
    typedef typename goo::dataflow::iProcessor::Ports::const_iterator T2;
    size_t operator()( const std::pair<T1, T2> & p ) const {
        return std::hash<T1>()(p.first)
             ^ std::hash<std::string>()(p.second->first)
             ;
    }
};
}  // namespace aux

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
    if( !itPortA->second.is_output() ) {
        emraise( badState, "Port \"%s\" of processor %p is not an output port."
               , aPortName.c_str(), &(nodeA.data()) );
    }
    if( nodeB.data().ports().end() == itPortB ) {
        emraise( noSuchKey, "Processor %p have no port named \"%s\"."
               , &(nodeB.data()), bPortName.c_str() );
    }
    if( !itPortB->second.is_input() ) {
        emraise( badState, "Port \"%s\" of processor %p is not an input port."
               , bPortName.c_str(), &(nodeB.data()) );
    }
    if( itPortA->second.type() != itPortB->second.type() ) {
        emraise( badCast, "Type mismatch: while trying to link port \"%s\":%s"
                " of processor %p with port \"%s\":%s of processor %p."
                , itPortA->first.c_str(), itPortA->second.type().name(), &(nodeA.data())
                , itPortB->first.c_str(), itPortB->second.type().name(), &(nodeB.data())
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
    _links.emplace( _links.size(), Link{ nodeA, nodeB, itPortA, itPortB } );
    _isCacheValid = false;
    return _links.size() - 1;
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

    // Compute values table
    //
    // Each value has it's physical representation identified by offset in
    // storage. With each value few links may be associated.
    //
    // To find out the values that has to be allocated, we have to consider all
    // connections between processors in a way similar to search in relational
    // table where following conditions steer the type of the value in storage:
    //  - links having the same destination port (on same processors) have to
    //  be considered as `join' value type;
    //  - links having the same source port (on same processors) have to
    //  be considered as `split' value type;
    //  - links with unique source and destination ports have to have their own
    //  unique values.
    //
    // Goo currently does not support `join' type of links. It seems to be
    // redundant for framework built in assumption of single message
    // propagation.
    typedef std::pair< dag::Node<iProcessor>*
                     , typename iProcessor::Ports::const_iterator> BoundPort_t;
    std::unordered_multiset< BoundPort_t
                           , aux::gfw_link_pair_hash> bySrc
                                                    , byDst;
    for( auto & l : _links ) {
        BoundPort_t src( &(l.second.nf), l.second.fp )
                  , dst( &(l.second.nt), l.second.tp )
                  ;
        bySrc.insert( src );
        byDst.insert( dst );
    }
    // Assure we have no `join' connection type
    for( auto & dstBucket : byDst ) {
        // Due to the fact our hashing function is not collision-free, we can
        // not rely solely on buckets and, thus, have to check for uniqueness
        // additionally within each bucket. If we suspect collision here, make
        // direct comparison of entries within a bucket.
        if( 1 < byDst.count( dstBucket ) ) {
            // Iterate over some entry in bucket first time
            for( auto it1 = byDst.begin( byDst.bucket(dstBucket) )
               ; byDst.end( byDst.bucket(dstBucket) ) != it1; ++it1 ) {
                // Iterate over some entry in bucket second time
                for( auto it2 = byDst.begin( byDst.bucket(dstBucket) )
                   ; byDst.end( byDst.bucket(dstBucket) ) != it2; ++it2 ) {
                    // 
                    if( it1 != it2
                     && it1->first == it2->first
                     && it1->second == it2->second ) {
                        emraise( malformedArguments, "Join link condition"
                                " revealed: at least two processors have their"
                                " outputs connected to input \"%s\" of"
                                " processor %p."
                               , it1->second->first.c_str()
                               , &(it1->first->data()) );
                    }
                }  // end of it2 loop
            }  // end of it1 loop
            std::cout << "XXX FALSE join condition suppressed." << std::endl;  // XXX
        } // end of bucket.count() > 1 if-clause
    } // end of by-bucket iteration
    // The `bySrc' set gives us exactly the set of values to allocate for
    // further usage
    std::vector<BoundPort_t> uniquePorts;
    for( auto & srcBucket : bySrc ) {
        // Now we have to count true unique src-ports to figure out the
        // proper storage layout (each unique src port have to cover a unique
        // value). If there are collisions (entries with matching hash, but
        // with distinct processor-port values), they have to be re-considered.
        if( 1 < bySrc.count( srcBucket ) ) {
            // Iterate over some entry in bucket first time
            for( auto it1 = bySrc.begin( bySrc.bucket(srcBucket) )
               ; bySrc.end( bySrc.bucket(srcBucket) ) != it1; ++it1 ) {
                // Iterate over some entry in bucket second time
                for( auto it2 = bySrc.begin( bySrc.bucket(srcBucket) )
                   ; bySrc.end( bySrc.bucket(srcBucket) ) != it2; ++it2 ) {
                    // That's a collision
                    if( it1->first != it2->first
                     || it1->second != it2->second ) {
                        //std::cerr << "XXX collision: "
                        //          << &(it1->first->data()) << ":\"" << it1->second->first << "\" vs "
                        //          << &(it2->first->data()) << ":\"" << it2->second->first << "\"" << std::endl
                        //          ;
                        //_TODO_  // collision revealed
                    }
                }  // end of it2 loop
            }  // end of it1 loop
            //std::cout << "XXX " << bySrc.count( srcBucket ) << " links on "
            //          << &(bySrc.begin( bySrc.bucket(srcBucket) )->first->data()) << ":\""
            //          <<   bySrc.begin( bySrc.bucket(srcBucket) )->second->first << "\"."
            //          << std::endl;
        }
        // Push back port description
        uniquePorts.push_back( *bySrc.begin( bySrc.bucket(srcBucket) ) );
        //std::cout << "XXX single link on "
        //          << &(bySrc.begin( bySrc.bucket(srcBucket) )->first->data()) << ":\""
        //          <<   bySrc.begin( bySrc.bucket(srcBucket) )->second->first << "\"."
        //          << std::endl;
    }

    size_t dataOffset = 0;

    // Compute out the worker storage and tiers structures
    std::list<Tier *> tiers;
    for(auto tierNodes : _cache.order) {
        tiers.push_back(new Tier(tierNodes));
        size_t nodeNum = 0;
        for(auto dagNodePtr : tierNodes) {
            auto nodePtr = static_cast<dag::Node<iProcessor>*>(dagNodePtr);
            iProcessor & p = nodePtr->data();
            for( auto portPtr : p.ports() ) {
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

