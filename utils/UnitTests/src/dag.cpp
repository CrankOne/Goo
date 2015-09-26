# include "utest.hpp"


/**@file dag.cpp
 * @brief Direct acyclic graph sorting tests.
 *
 * DAG perform a basis for unit testing functionality, so it should
 * be presented in some way. Currently there is no point to include
 * this module in UT graph as malfunction of this module causes entire
 * UT system to be malfunctional.
 * */

GOO_UT_BGN( serialization )

{
    typedef DAG<char> NumberHoldingGraph;
    struct DepDecl {
        char name;
        char deps[9];
        NumberHoldingGraph::Node * n;
        } deps[] = {
            { '1', "6"      },
            { '2', "6"      },
            { '3', "7"      },
            { '4', ""       },
            { '5', "389"    },
            { '6', "9"      },
            { '7', "98"     },
            { '8', "0"      },
            { '9', "0"      },
            { '0', ""       },
        };
    NumberHoldingGraph ng;
    std::unordered_map<char, NumberHoldingGraph::Node *> nodesIndex;
    // Initialize nodes
    for( uint8_t i = 0; i < sizeof(deps)/sizeof(DepDecl); ++i ) {
        DepDecl * cnDataPtr = deps + i;
        cnDataPtr->n = ng.insert_data( &(cnDataPtr->name) );
        nodesIndex[cnDataPtr->name] = cnDataPtr->n;
    }
    // Set-up nodes dependencies.
    for( uint8_t i = 0; i < sizeof(deps)/sizeof(DepDecl); ++i ) {
        DepDecl * cnDataPtr = deps + i;
        for( char * c = cnDataPtr->deps; *c != '\0'; c++ ) {
            cnDataPtr->n->depends_from( nodesIndex[*c] );
        }
    }
}



GOO_UT_END( serialization )

