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
    # ifdef NDEBUG
    _namesByNodes.emplace(ir.first->second, name);
    # else
    auto iir = _namesByNodes.emplace(ir.first->second, name);
    assert(iir.second);  // assure reverse index insertion
    # endif
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
    _cache.layoutMap.clear();
    _cache.dataSize = 0;
}

void
Framework::_recache() const {
    _free_cache();
    // Compute order of execution
    _cache.order = dag::dfs(_nodes);
    for( auto tierDescription : _cache.order ) {
        _cache.tiers.push_back( new Tier(tierDescription) );
    }
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
    // Initialize data layout map: each output (or bidirectional) port has to
    // have it's own physical data representation
    _cache.dataSize = 0;  // cumulatevely incrementing
    for( auto outPortPair : _cache.bySrcLinked ) {
        auto rng = _cache.bySrcLinked.equal_range( outPortPair.first );
        assert( rng.first != _cache.bySrcLinked.end() );
        for( auto it = rng.first; rng.second != it; ++it ) {
            _cache.layoutMap.emplace( it->second, _cache.dataSize );
        }
        _cache.dataSize += outPortPair.first.second->second.data_size();
    }
    // Recaching done.
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

