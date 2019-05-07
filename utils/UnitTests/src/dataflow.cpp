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

# include "utest.hpp"
# include "goo_dataflow/framework.hpp"
# include "goo_dataflow/worker.hpp"

# include <fstream>  // XXX file i/o for .dot debug

namespace gdf = goo::dataflow;

// TODO: make it stateless
/// A testing single-output stateless processor, generating uniform random
/// number in [1:6] interval.
class Dice : public gdf::iProcessor {
protected:
    virtual gdf::PSC _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<int>("value", rand()%6 + 1);
        return 0;
    }
public:
    Dice() {
        out_port<int>("value");
    }
};

/// Sums 6 integer numbers.
class Sum6 : public gdf::iProcessor {
protected:
    virtual gdf::PSC _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<int>("S", vm.get<int>("x1") + vm.get<int>("x2")
                       + vm.get<int>("x3") + vm.get<int>("x4")
                       + vm.get<int>("x5") + vm.get<int>("x6") );
        return 0;
    }
public:
    Sum6() {
        in_port<int>("x1"); in_port<int>("x2"); in_port<int>("x3");
        in_port<int>("x4"); in_port<int>("x5"); in_port<int>("x6");

        out_port<int>("S");
    }
};

/// Sums 2 integer numbers.
class Sum2 : public gdf::iProcessor {
protected:
    virtual gdf::PSC _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<int>("c", vm.get<int>("a") + vm.get<int>("b") );
        return 0;
    }
public:
    Sum2() {
        in_port<int>("a");    in_port<int>("b");

        out_port<int>("c");
    }
};

/// Compares two numbers; writes number of (mis-)matches.
class Compare : public gdf::iProcessor {
private:
    size_t _match, _mismatch;
protected:
    virtual gdf::PSC _V_eval( gdf::ValuesMap & vm ) override {
        if( vm.get<int>("A") == vm.get<int>("B") ) {
            ++_match;
        } else {
            ++_mismatch;
        }
        return 0;
    }
public:
    Compare() : _match(0), _mismatch(0) {
        in_port<int>("A");    in_port<int>("B");
    }

    size_t total() const { return _match + _mismatch; }
    size_t n_match() const { return _match; }
    size_t n_mismatch() const { return _match; }
};

GOO_UT_BGN( Dataflow, "Dataflow framework" ) {
    gdf::Framework fw;
    // All processors except "Compare" are statelss, so we can safely populate
    // single instances among few nodes in framework.
    Dice dice;
    Sum2 sum2;
    Sum6 sum6;
    // Compare is stateful, however we'll need just one instance...
    Compare cmp;

    // Emplace 6 "dices" anmed as "Dice #N".
    for( int i = 0; i < 6; ++i ) {
        char bf[32];
        snprintf( bf, sizeof(bf), "Dice #%d", i+1 );
        fw.impose( bf, dice );
    }
    // Emplace 5 pairwise summators
    for( int i = 0; i < 5; ++i ) {
        char bf[32];
        snprintf( bf, sizeof(bf), "Sum-2 #%d", i+1 );
        fw.impose( bf, sum2 );
    }
    // Emplace 6-wise summator
    fw.impose( "Sum-6", sum6 );
    fw.impose( "Compare", cmp );
    {
        // Connect dices to 6-input sum
        fw.precedes( "Dice #1",  "value", "Sum-6",    "x1" );
        fw.precedes( "Dice #2",  "value", "Sum-6",    "x2" );
        fw.precedes( "Dice #3",  "value", "Sum-6",    "x3" );
        fw.precedes( "Dice #4",  "value", "Sum-6",    "x4" );
        fw.precedes( "Dice #5",  "value", "Sum-6",    "x5" );
        fw.precedes( "Dice #6",  "value", "Sum-6",    "x6" );
        // Connect dices and pairwise sums
        fw.precedes( "Dice #2",  "value", "Sum-2 #1", "a" );
        fw.precedes( "Dice #3",  "value", "Sum-2 #1", "b" );
        fw.precedes( "Dice #5",  "value", "Sum-2 #2", "a" );
        fw.precedes( "Dice #6",  "value", "Sum-2 #2", "b" );
        fw.precedes( "Dice #1",  "value", "Sum-2 #3", "a" );
        fw.precedes( "Sum-2 #1", "c"    , "Sum-2 #3", "b" );
        fw.precedes( "Dice #4",  "value", "Sum-2 #4", "a" );
        fw.precedes( "Sum-2 #2", "c"    , "Sum-2 #4", "b" );
        fw.precedes( "Sum-2 #3", "c"    , "Sum-2 #5", "a" );
        fw.precedes( "Sum-2 #4", "c"    , "Sum-2 #5", "b" );
        // Compare pairwise sum result with 6-sum
        fw.precedes( "Sum-2 #5", "c",     "Compare", "A" );
        fw.precedes( "Sum-6",    "S",     "Compare", "B" );
    }
    # if 0
    fw.generate_dot_graph(os);
    # else
    std::ofstream dotF;
    dotF.open("/tmp/one.dot");
    fw.generate_dot_graph(dotF);
    dotF.close();
    # endif
    gdf::Worker w1( fw );
    std::thread t1( &gdf::Worker::run
                  , &w1 );
    t1.join();
    os << "Control:" << std::endl
       << " - match:" << cmp.n_match() << std::endl
       << " - mismatch:" << cmp.n_mismatch() << std::endl
       << " - total:" << cmp.total() << std::endl
       ;
    _ASSERT( 0 == cmp.n_mismatch()
           , "Mismatch values revealed." );
    //_ASSERT( cmp.n_match() == 1, "Wrong number of values have passed"
    //        " the comparison processor." );
} GOO_UT_END( Dataflow, "Bitset", "DFS_DAG" )
