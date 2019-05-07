# include "goo_dataflow/framework.hpp"

# include <iomanip>

namespace goo {
namespace dataflow {

# if 0
namespace aux {
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
}  // namespace aux
# endif

bool
Framework::Cache::BoundLinkLess::operator()( const BoundPort_t & a
               , const BoundPort_t & b ) const {
    return (a.first < b.first)
        || (a.first == b.first && a.second->first.compare(b.second->first) < 0)
        ;
}

Framework::Framework() : _isCacheValid(false) {}

Framework::~Framework() {
    _free_cache();
    for( auto nPtr : _nodes ) {
        delete nPtr;
    }
}

Framework::ExecNode *
Framework::impose( iProcessor & p ) {
    auto en = new ExecNode(p);
    _nodes.insert(en);
    _invalidate_cache();
    return en;
}

Framework::ExecNode *
Framework::impose( const std::string & name, iProcessor & p ) {
    auto it = _nodesByName.find(name);
    if( _nodesByName.end() != it ) {
        emraise( nonUniq
               , "Unable to insert node over a processor %p named"
                 " \"%s\" since this name is already assigned to"
                 " processor %p.", &p, name.c_str(), it->second );
    }
    auto ir = _nodesByName.emplace(name, impose(p));
    if( ! ir.second ) {
        emraise( badState
               , "Unable to insert node named \"%s\" over processor %p."
               , name.c_str(), &p );
    }
    auto iir = _namesByNodes.emplace(ir.first->second, name);
    assert(iir.second);  // assure reverse index insertion
    return ir.first->second;
}

std::pair< typename iProcessor::Ports::const_iterator
         , typename iProcessor::Ports::const_iterator>
Framework::_assure_link_valid(
                     ExecNode & nodeA, const std::string & aPortName
                   , ExecNode & nodeB, const std::string & bPortName
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
Framework::precedes( ExecNode * a, const std::string & aPortName
                   , ExecNode * b, const std::string & bPortName ) {
    ExecNode & nodeA = *a
           , & nodeB = *b;
    auto [itPortA, itPortB] = _assure_link_valid( nodeA, aPortName, nodeB, bPortName );
    nodeA.depends_on( nodeB );
    _links.emplace( _links.size(), Link{ nodeA, nodeB, itPortA, itPortB } );
    _invalidate_cache();
    return _links.size() - 1;
}

void
Framework::_free_cache() const {
    _cache.order.clear();
    for( auto tierPtr : _cache.tiers ) {
        delete tierPtr;
    }
    _cache.tiers.clear();
    _cache.bySrcLinked.clear();
    _cache.byDstLinked.clear();
    _cache.bySrcAll.clear();
    _cache.byDstAll.clear();
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
    
    // Fill source port -> LinkID map
    std::transform( _links.begin(), _links.end()
                  , std::inserter(_cache.bySrcLinked, _cache.bySrcLinked.begin())
                  , [](const std::pair<size_t, Link> & p )
                    { return std::pair<Cache::BoundPort_t, size_t>
                            ( Cache::BoundPort_t(&(p.second.nf), p.second.fp)
                            , p.first
                            ); }
                  );
    // Fill dest port -> LinkID map
    std::transform( _links.begin(), _links.end()
                  , std::inserter(_cache.byDstLinked, _cache.byDstLinked.begin())
                  , [](const std::pair<size_t, Link> & p )
                    { return std::pair<Cache::BoundPort_t, size_t>
                            ( Cache::BoundPort_t(&(p.second.nt), p.second.tp)
                            , p.first
                            ); }
                  );
    // Fill sets of source/dest links sorted by i/o feature (sets may
    // intersect)
    for( auto dagNPtr : _nodes ) {
        auto nPtr = static_cast<ExecNode*>(dagNPtr);
        for( auto portIt = nPtr->data().ports().cbegin()
           ; portIt != nPtr->data().ports().cend(); ++portIt ) {
            assert( portIt->second.is_input() || portIt->second.is_output() );
            if( portIt->second.is_output() ) {
                _cache.bySrcAll.insert( Cache::BoundPort_t( nPtr, portIt ) );
            }
            if( portIt->second.is_input() ) {
                _cache.byDstAll.insert( Cache::BoundPort_t( nPtr, portIt ) );
            }
        }
    }
    // Initialize data layout map: each output (or bidirectional) port has to
    // have it's own physical data representation
    size_t dataOffset = 0
         , dataSize;

    //std::cout << std::endl
    //          << " Offset | Output port       | nLinks | size" << std::endl
    //          << "--------+-------------------+--------+--------" << std::endl; // XXX
    for( auto outPortPair : _cache.bySrcAll ) {
        dataSize = outPortPair.second->second.data_size();
        //bySrcLinked.find( it. );
        //_TODO_ //_cache.layoutMap.emplace( outPortPair, dataOffset );
        //{  // debug out, XXX
        //    std::cout << std::setw(7) << dataOffset << " | "
        //              << std::setw(9) << outPortPair.first << "::"
        //              << std::left << std::setw(6) << outPortPair.second->first << std::right << " | "
        //              << std::setw(6) << _cache.bySrcLinked.count(outPortPair) << " | "
        //              << std::setw(6) << std::left << dataSize << std::right
        //              << std::endl;
        //}
        dataOffset += dataSize;
    }
    _isCacheValid = true;
}

void
Framework::generate_dot_graph( std::ostream & os ) const {

    os << "digraph g {" << std::endl;
    for( auto dagNodePtr : _nodes ) {
        auto n = static_cast<const ExecNode*>(dagNodePtr);
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
                for( auto portIt = n->data().ports().cbegin()
                   ; n->data().ports().cend() != portIt; ++portIt ) {
                    auto pp = *portIt;
                    if( ! pp.second.is_input() ) continue;
                    os << "      <TD SIDES=\"R\" PORT=\"" << pp.first;
                    auto dstCnctdIt = get_cache().byDstLinked.find( Cache::BoundPort_t( const_cast<ExecNode*>(n), portIt ) );
                    if( get_cache().byDstLinked.end() == dstCnctdIt ) {
                        os << "\" BGCOLOR=\"red";
                    }
                    os << "\"><FONT POINT-SIZE=\"10\">"
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
            char bf[128];
            auto procNameIt = _namesByNodes.find(const_cast<ExecNode*>(n));
            if( _namesByNodes.end() == procNameIt ) {
                snprintf( bf, sizeof(bf), "%p", n );
            } else {
                snprintf( bf, sizeof(bf), "%s", procNameIt->second.c_str() );
            }
            os << "   <TD BORDER=\"1\" ALIGN=\"CENTER\" CELLPADDING=\"3\">"
               << "<U>" << typeid(n->data()).name() << "</U><BR/>"
               << "<B>" << bf << "</B><BR/>"
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
                for( auto portIt = n->data().ports().cbegin()
                   ; n->data().ports().cend() != portIt; ++portIt ) {
                    auto pp = *portIt;
                    if( ! pp.second.is_output() ) continue;
                    os << "      <TD SIDES=\"R\" PORT=\"" << pp.first;
                    auto srcCnctdIt = get_cache().bySrcLinked.find( Cache::BoundPort_t( const_cast<ExecNode*>(n), portIt ) );
                    if( get_cache().bySrcLinked.end() == srcCnctdIt ) {
                        os << "\" BGCOLOR=\"yellow";
                    }
                    os << "\"><FONT POINT-SIZE=\"10\">"
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
           << "node" << &(l.nt) << ":" << l.tp->first << ":n"
           //<< " [label=\""
           //<< _cache.layoutMap[Cache::BoundPort_t(&l.nf, l.fp)]
           << "\"];" << std::endl;
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

