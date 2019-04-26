/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# include <cstring>
# include "utest.hpp"

/**@file dag_dfs.cpp
 * @brief Depth-first for direct acyclic graph test.
 *
 * DAG perform a basis for unit testing functionality, so it should
 * be presented in some way. Currently there is no point to include
 * this module in UT graph as malfunction of this module causes entire
 * UT system to be malfunctional.
 * */

typedef std::unordered_map<char, goo::dag::Node<char>*> NumberHoldingGraph;

struct DepDecl {
    char name;
    char deps[9];
    goo::dag::Node<char> * n;
};

static const char _static_manualHelpInstructions[] = 
R"(In order to manually check correctness,
one can draw the nodes in following form:
    0
  9   8
 6  7
1 2 3 4 5
and denote the following dependency relations:)"
;  // Note the C++11 multiline string syntax

//
// local testing routines:
void
dump_order( std::ostream & os
          , const goo::dag::Order & order ) {
    for( auto tier : order ) {
        os << "{";
        for( auto nPtr : tier ) {
            os << static_cast<goo::dag::Node<char>*>(nPtr)->data() << ", ";
        }
        os << "}";
    }
}

uint16_t
char_to_n( char c ) {
    return c - '0';
}

bool
is_nth_resolved( const uint16_t indicators, uint8_t n ) {
    return indicators & (0x1 << n );
}

void
resolve_nth( uint16_t & indicators, uint8_t n ) {
    indicators |= (0x1 << n );
}

void
check_resolution_chain( const DepDecl * arr,
                        const goo::dag::Order & order,
                        char target  // 0 means 'no target'
                        ) {
    // used to check order correctness. True bit means 'resolved'.
    uint16_t visitIndicatorBits = 0x0;
    for( auto tier : order ) {
        for( auto nPtr : tier ) {
            const char cNodeChar = static_cast<goo::dag::Node<char>*>(nPtr)->data();
            for( const DepDecl * cDecl = arr; cDecl->name != '\0'; ++cDecl ) {
                if( strchr(cDecl->deps, cNodeChar) ) {
                    if( !is_nth_resolved( visitIndicatorBits, char_to_n(cDecl->name) ) ) {
                        std::stringstream ss;
                        dump_order( ss, order );
                        emraise( uTestFailure,
                                "Ordered node refers to unresolved node; target: %c, order: %s, problem on: %c w dep %c.",
                                target ? target : '!',
                                ss.str().c_str(),
                                cNodeChar,
                                cDecl->name
                            );
                    }
                }
            }
            resolve_nth( visitIndicatorBits, char_to_n( cNodeChar ) );
        }
    }
    // Check, if target is resolved:
    if( !target ) {
        // no target -- then all 10 bits should be set to 1; even an orphants
        for( const DepDecl * cDecl = arr; cDecl->name != '\0'; ++cDecl ) {
            uint8_t n = char_to_n( cDecl->name );
            if( ! (visitIndicatorBits & (0x1 << n)) ) {
                std::stringstream ss;
                dump_order( ss, order );
                emraise( uTestFailure, "Node %c unresolved for DFS DAG traversal. Order is: %s.",
                        cDecl->name,
                        ss.str().c_str()
                    );
            }
        }
    } else {
        // Check only target.
        uint8_t targetN = char_to_n( target );
        if( ! (visitIndicatorBits & (0x1 << targetN)) ) {
            std::stringstream ss;
            dump_order( ss, order );
            emraise( uTestFailure, "Target %c remained unvisited. Order is: %s.",
                    target,
                    ss.str().c_str()
                );
        }
    }
}

GOO_UT_BGN( DFS_DAG, "Direct acyclic graph depth-first search tests" )

{  // Basic DAG
    DepDecl deps[] = {
            // #   dep of:
            { '1', "6"      }, { '2', "60"     },
            { '3', "7"      }, { '4', ""       },
            { '5', "389"    }, { '6', "9"      },
            { '7', "982"    }, { '8', "10"     },
            { '9', "0"      }, { '0', ""       },
            { '\0', "sentinel" },
        };
    os << "Now, the basic DAG testing procedures will be invoked:" << std::endl;
    NumberHoldingGraph ng;
    // Initialize nodes
    for( DepDecl * c = deps; '\0' != c->name; ++c ) {
        c->n = new goo::dag::Node<char>(c->name);
        ng.emplace( c->name, c->n );
    }
    os << _static_manualHelpInstructions << std::endl;
    // Set-up nodes dependencies.
    for( DepDecl * c = deps; '\0' != c->name; ++c ) {
        os << " node '" << c->name << "' is required for: ";
        for( char * cd = c->deps; *cd != '\0'; cd++ ) {
            auto depIt = ng.find(*cd);
            assert( ng.end() != depIt );
            c->n->depends_on( *(depIt->second) );
            os << *cd << ",";
        }
        os << std::endl;
    }

    std::unordered_set<goo::dag::DAGNode*> nodes;
    std::transform( ng.begin(), ng.end()
                  , std::inserter( nodes, nodes.begin() )
                  , [](std::pair<char, goo::dag::Node<char> *> p)
                    { return p.second; });

    os << "Now get some fun with DAG:" << std::endl;
    // Fun:
    goo::dag::Order order;
    os << " - generic DFS" << std::endl;
    order = goo::dag::dfs( nodes );
    check_resolution_chain( deps, order, 0 );  // check
    dump_order( os, order );
    os << std::endl;
    order.clear();

    os << " - minimal necessary chain search" << std::endl;
    os << " -- trivial (orphant) node" << std::endl;
    goo::dag::visit( *((deps + 3)->n), order );
    check_resolution_chain( deps, order, '4' );  // check
    dump_order( os, order );
    os << std::endl;
    order.clear();
    for( auto n : nodes ) n->reset_dfs_descriptor();

    os << " -- middle-length" << std::endl;
    goo::dag::visit( *((deps + 7)->n), order );
    check_resolution_chain( deps, order, '8' );  // check
    dump_order( os, order );
    os << std::endl;
    order.clear();
    for( auto n : nodes ) n->reset_dfs_descriptor();

    os << " -- long-length" << std::endl;
    goo::dag::visit( *((deps + 8)->n), order );
    check_resolution_chain( deps, order, '9' );  // check
    dump_order( os, order );
    os << std::endl;
    order.clear();
    for( auto n : nodes ) n->reset_dfs_descriptor();

    os << " -- full-length" << std::endl;
    goo::dag::visit( *((deps + 9)->n), order );
    check_resolution_chain( deps, order, '0' );  // check
    dump_order( os, order );
    os << std::endl;
    order.clear();
    for( auto n : nodes ) n->reset_dfs_descriptor();

    os << "ok!" << std::endl;
}

GOO_UT_END( DFS_DAG )
