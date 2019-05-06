# include "goo_dataflow/framework.hpp"

# include <iomanip>

namespace goo {
namespace dataflow {

namespace aux {
typedef std::pair< goo::dag::Node<goo::dataflow::iProcessor> *
                 , typename goo::dataflow::iProcessor::Ports::const_iterator> BoundPort_t;

// This hashing function probably does not provide hash being unique enough to
// rely on it as collision-free, though providing us some boost with
// unique-finding routines need in Framework::recache() method. Moreover,
// the resulting hash value depends on pointer.
struct gfw_link_pair_hash {
    typedef goo::dag::Node<goo::dataflow::iProcessor> * T1;
    typedef typename goo::dataflow::iProcessor::Ports::const_iterator T2;
    size_t operator()( const BoundPort_t & p ) const {
        return std::hash<T1>()(p.first)
             ^ std::hash<std::string>()(p.second->first)
             ;
    }
};

struct gfw_link_pair_less {
    bool operator()( const BoundPort_t & a
                   , const BoundPort_t & b ) const {
        return (a.first < b.first)
            || (a.first == b.first && a.second->first.compare(b.second->first) < 0)
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
    
    std::multimap< aux::BoundPort_t
                 , size_t
                 , aux::gfw_link_pair_less > bySrcLinked
                                           , byDstLinked
                                           ;
    std::set<aux::BoundPort_t, aux::gfw_link_pair_less> bySrcAll, byDstAll;
    // Fill source port -> LinkID map
    std::transform( _links.begin(), _links.end()
                  , std::inserter(bySrcLinked, bySrcLinked.begin())
                  , [](const std::pair<size_t, Link> & p )
                    { return std::pair<aux::BoundPort_t, size_t>
                            ( aux::BoundPort_t(&(p.second.nf), p.second.fp)
                            , p.first
                            ); }
                  );
    // Fill dest port -> LinkID map
    std::transform( _links.begin(), _links.end()
                  , std::inserter(byDstLinked, byDstLinked.begin())
                  , [](const std::pair<size_t, Link> & p )
                    { return std::pair<aux::BoundPort_t, size_t>
                            ( aux::BoundPort_t(&(p.second.nf), p.second.fp)
                            , p.first
                            ); }
                  );
    // Fill sets of source/dest links sorted by i/o feature (sets may
    // intersect)
    for( auto dagNPtr : _nodes ) {
        auto nPtr = static_cast<dag::Node<iProcessor>*>(dagNPtr);
        for( auto portIt = nPtr->data().ports().cbegin()
           ; portIt != nPtr->data().ports().cend(); ++portIt ) {
            assert( portIt->second.is_input() || portIt->second.is_output() );
            if( portIt->second.is_output() ) {
                bySrcAll.insert( aux::BoundPort_t( nPtr, portIt ) );
            }
            if( portIt->second.is_input() ) {
                byDstAll.insert( aux::BoundPort_t( nPtr, portIt ) );
            }
        }
    }
    // Initialize data layout map: each output (or bidirectional) port has to
    // have it's own physical data representation
    size_t dataOffset = 0
         , dataSize;
    std::map<aux::BoundPort_t, size_t, aux::gfw_link_pair_less> layoutMap;

    //std::cout << std::endl
    //          << " Offset | Output port       | nLinks | size" << std::endl
    //          << "--------+-------------------+--------+--------" << std::endl; // XXX
    for( auto outPortPair : bySrcAll ) {
        dataSize = outPortPair.second->second.data_size();
        layoutMap.emplace( outPortPair, dataOffset );
        //{  // debug out, XXX
        //    std::cout << std::setw(7) << dataOffset << " | "
        //              << std::setw(9) << outPortPair.first << "::"
        //              << std::left << std::setw(6) << outPortPair.second->first << std::right << " | "
        //              << std::setw(6) << bySrcLinked.count(outPortPair) << " | "
        //              << std::setw(6) << std::left << dataSize << std::right
        //              << std::endl;
        //}
        dataOffset += dataSize;
    }
}

void
Framework::generate_dot_graph( std::ostream & os ) const {
    os << "digraph g {" << std::endl;
    for( auto dagNodePtr : _nodes ) {
        auto n = static_cast<const dag::Node<iProcessor>*>(dagNodePtr);
        std::map<std::string, PortInfo> inPorts, outPorts;
        for( auto pp : n->data().ports() ) {
            if( pp.second.is_input() ) {
                inPorts.insert(pp);
            }
            if( pp.second.is_output() ) {
                outPorts.insert(pp);
            }
        }
        os << " node" << dagNodePtr << " [ shape=component, margin=0, label=<"
           << "<TABLE BORDER=\"0\" CELLBORDER=\"0\" CELLSPACING=\"0\" CELLPADDING=\"0\">" << std::endl;
        if( ! inPorts.empty() ) {
            os << "  <TR>" << std::endl
               << "   <TD>" << std::endl
               << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"0\">" << std::endl
               << "     <TR>" << std::endl;
            {  // input ports
                for( auto pp : inPorts ) {
                    if( ! pp.second.is_input() ) continue;
                    // TODO BGCOLOR="red" if not connected, "grey" if not connected
                    // and optional
                    os << "      <TD SIDES=\"R\" PORT=\"" << pp.first << "\"><FONT POINT-SIZE=\"10\">"
                       << pp.first
                       << "</FONT></TD>" << std::endl;
                }
            }
            os << "     </TR>" << std::endl
               << "    </TABLE>" << std::endl
               << "   </TD>" << std::endl
               << "  </TR>" << std::endl;
        }
        os << "  <TR>" << std::endl;
        {  // processor credentials
            os << "   <TD BORDER=\"1\" ALIGN=\"CENTER\" CELLPADDING=\"3\">"
               << "<U>" << typeid(n->data()).name() << "</U><BR/>"
               << "<B>" << dagNodePtr << "</B><BR/>"  // TODO: label, if has
               << &(n->data())
               << "</TD>" << std::endl;
        }
        os << "  </TR>" << std::endl;
        if( ! outPorts.empty() ) {
            os << "  <TR>" << std::endl
               << "   <TD>" << std::endl
               << "    <TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"0\">" << std::endl
               << "     <TR>" << std::endl;
            {  // output ports
                for( auto pp : n->data().ports() ) {
                    if( ! pp.second.is_output() ) continue;
                    // TODO BGCOLOR="grey" if not connected
                    os << "      <TD SIDES=\"R\" PORT=\"" << pp.first << "\"><FONT POINT-SIZE=\"10\">"
                       << pp.first
                       << "</FONT></TD>" << std::endl;
                }
            }
            os << "     </TR>" << std::endl
               << "    </TABLE>" << std::endl
               << "   </TD>" << std::endl
               << "  </TR>" << std::endl;
        }
        os << " </TABLE>>];" << std::endl;
    }
    for( auto linkPair : _links ) {
        const Link & l = linkPair.second;
        os << " node" << &(l.nf) << ":" << l.fp->first << ":s -> "
           << "node" << &(l.nt) << ":" << l.tp->first << ":n;"
           << std::endl;
    }
    os << "}" << std::endl;
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

