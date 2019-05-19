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


/**@file dataflow.cpp
 * @brief Basic dataflow facility unit test sources.
 *
 * @TODO: assure that DAG is processed in a really semi-parallel mode, by
 * introducing some checks to work log.
 * */

# include "utest.hpp"
# include "goo_dataflow/framework.hpp"
# include "goo_dataflow/worker.hpp"

// Enable this to generate a dedicated .dot filefor dev debugging
//# define _m_DEV_WRITE_DOT_FILE  "/tmp/gdf_example.dot"
// Enable this to perform a single DAG traversal for dev purposes
//# define _m_DEV_SINGLE_THREADED_DAG_TRAV

# include <iomanip>

# ifdef _m_DEV_WRITE_DOT_FILE
#   include <fstream>
# endif

namespace gdf = goo::dataflow;

// TODO: explicitly mark it as a stateless processor
/// A testing single-output stateless processor, generating uniform random
/// number in [1:6] interval.
class Dice : public gdf::iProcessor {
protected:
    virtual gdf::PSC _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<int>("value", rand()%6 + 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
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
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
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
    size_t n_mismatch() const { return _mismatch; }
};

// Used to control over elapsed time and worker events.
struct ParallelEvent {
    uint8_t type;
    size_t nProc, nTier, nWorker;
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
    # ifdef _m_DEV_WRITE_DOT_FILE  // enables .dot file dump for dev checks
    std::ofstream dotF;
    dotF.open(_m_DEV_WRITE_DOT_FILE);
    fw.generate_dot_graph(dotF);
    dotF.close();
    # else
    fw.generate_dot_graph(os);
    # endif
    # ifdef _m_DEV_SINGLE_THREADED_DAG_TRAV
    gdf::Worker w1( fw );
    w1.run();
    # else
    const size_t nThreads = std::thread::hardware_concurrency();
    std::thread * ts[nThreads];
    gdf::Worker * ws[nThreads];
    for( size_t nThread = 0; nThread < nThreads; ++nThread ) {
        ws[nThread] = new gdf::Worker( fw );
        ts[nThread] = new std::thread( &gdf::Worker::run, ws[nThread] );
    }
    std::multimap<std::clock_t, ParallelEvent> paes;
    for( size_t nThread = 0
       ; nThread < nThreads
       ; ++nThread ) {
        ts[nThread]->join();
        delete ts[nThread];
        auto workLog = ws[nThread]->log();
        std::transform( workLog.begin(), workLog.end()
                      , std::inserter(paes, paes.begin())
                      , [&nThread](const gdf::Worker::LogEntry & le) {
                        return std::pair<clock_t, ParallelEvent>( le.time, ParallelEvent{
                                le.type, le.nProc, le.nTier, nThread
                            } );
                      } );
        delete ws[nThread];
    }
    
    os << "Dataflow:" << std::endl
       << "  time |";
    for( size_t nThread = 0; nThread < nThreads; ++nThread ) {
        os << " worker #" << std::setw(2) << std::left << nThread << " |";
    }
    os << std::endl;
    std::clock_t start = paes.begin()->first;
    size_t cnt;
    for( auto pae : paes ) {
        os << std::setw(6) << std::right << pae.first - start << " | ";
        for( cnt = 0; cnt < pae.second.nWorker; ++cnt ) {
            os << "           | ";
        }
        {
            std::stringstream ss;
            ss << pae.second.nTier
               << "/"
               << pae.second.nProc
               << "-" << ( pae.second.type == gdf::Worker::LogEntry::execStarted
                         ? "bgn" : "end");
            os << std::setw(10) << ss.str() << " | ";
        }
        for( ++cnt ; cnt < nThreads; ++cnt ) {
            os << "           | ";
        }
        os << std::endl;
    }

    os << "Control:" << std::endl
       << " - match:" << cmp.n_match() << std::endl
       << " - mismatch:" << cmp.n_mismatch() << std::endl
       << " - total:" << cmp.total() << std::endl
       ;
    _ASSERT( 0 == cmp.n_mismatch()
           , "Mismatch values revealed." );
    _ASSERT( cmp.total() == nThreads, "Wrong number of values have passed"
           " the comparison processor: %zu (%zu expected)."
           , cmp.total(), nThreads);
    # endif
} GOO_UT_END( Dataflow, "Bitset", "DFS_DAG" )
