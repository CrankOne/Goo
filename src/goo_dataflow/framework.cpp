# include "goo_dataflow/framework.hpp"

# include <sqlite3.h>

namespace goo {
namespace dataflow {

# if ! SQLITE3_FOUND
namespace aux {
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
# else
void
Framework::_build_values_map( const std::unordered_map<size_t, Link> & linksSet ) {
	sqlite3 * db;
	char * zErrMsg = 0;
    char pSQLVar[1024];
	int rc;
	rc = sqlite3_open( "/tmp/links.sqlite3", &db );
	if( rc ) {
		emraise( badState, "Unable to open sqlite3 database; rc=%d.", rc );
	}

    # if 0
	pSQL[0] = "create table myTable (FirstName varchar(30), LastName varchar(30), Age smallint, Hometown varchar(30), Job varchar(30))";
	pSQL[1] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Peter', 'Griffin', 41, 'Quahog', 'Brewery')";
	pSQL[2] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Lois', 'Griffin', 40, 'Newport', 'Piano Teacher')";
	pSQL[3] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Joseph', 'Swanson', 39, 'Quahog', 'Police Officer')";
	pSQL[4] = "insert into myTable (FirstName, LastName, Age, Hometown, Job) values ('Glenn', 'Quagmire', 41, 'Quahog', 'Pilot')";
	pSQL[5] = "select * from myTable";
	pSQL[6] = "delete from myTable"; 
	pSQL[7] = "drop table myTable";
	for(size_t i = 0; i < sizeof(pSQL)/sizeof(*pSQL); ++i) {
		rc = sqlite3_exec(db, pSQL[i], NULL, 0, &zErrMsg);
		if( rc != SQLITE_OK ) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
			sqlite3_free(zErrMsg);
			break;
		}
	}
    # endif

    // Create & fill links table
    rc = sqlite3_exec( db, "CREATE TABLE links ( "
                "LinkID INT,"
                "FromProc UNSIGNED BIG INT,"
                "FromPort VARCHAR(128),"
                "ToProc UNSIGNED BIG INT,"
                "ToPort VARCHAR(128),"
                "Value INTEGER"
            " )", NULL, NULL, &zErrMsg );
    if( SQLITE_OK != rc ) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_free(zErrMsg);
    }
    for( auto lp : linksSet ) {
        snprintf( pSQLVar, sizeof(pSQLVar)
                , "INSERT INTO links (LinkID, FromProc, FromPort, ToProc, ToPort)"
                " VALUES (%zu, %p, '%s', %p, '%s')"
                , lp.first
                , &(lp.second.nf), lp.second.fp->first.c_str()
                , &(lp.second.nt), lp.second.tp->first.c_str() );
        if( SQLITE_OK != sqlite3_exec( db, pSQLVar, NULL, NULL, &zErrMsg ) ) {
            std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    		sqlite3_free(zErrMsg);
            break;
        }
    }
    // Find splits:
    rc = sqlite3_exec( db,
        "SELECT LinkID, FromProc, FromPort, count(*)"
        " FROM links"
        " GROUP BY FromProc, FromPort"
        " HAVING count(*) > 1"
        " ORDER BY LinkID;", NULL, NULL, &zErrMsg );
    if( SQLITE_OK != rc ) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
		sqlite3_free(zErrMsg);
    }

	sqlite3_close(db);
}

# endif

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
    //  To accomplish this we currently involve some SQL routines... That have
    //  to be, perhaps, substituted with pure C++ algorithm further.
    # if ! SQLITE3_FOUND
    std::unordered_map<std::pair< dag::Node<iProcessor>*
                                , typename iProcessor::Ports::const_iterator>
                      , size_t, aux::gfw_link_pair_hash> joinValues
                                                       , splitValues
                                                       , standaloneValues
                                                       ;

    for( auto lPtr : _links ) {
        auto from = std::pair< dag::Node<iProcessor>*
                             , typename iProcessor::Ports::const_iterator>( &(lPtr->nf), lPtr->fp )
           //,   to = std::pair< dag::Node<iProcessor>*
           //                  , typename iProcessor::Ports::const_iterator>( &(lPtr->nf), lPtr->fp )
           ;
        auto saIt = standaloneValues.find( from );
        if( standaloneValues.end() != saIt ) {
            // having at least two links that goes out of the same port: split
            // candidate
        }
        //dataOffset += linkPtr->from()->second.typeInfo->typeSize;
    }

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
    # else
    _build_values_map( _links );
    # endif
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

