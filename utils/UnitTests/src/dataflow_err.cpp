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


/**@file dataflow_err.cpp
 * @brief Tests for dataflow error-treatment capabilites.
 *
 * This fixture tests all the return codes possible during the dataflow
 * processing and their effects on propagation as well as basic error-recovery
 * mechanics provided by dataflow framework.
 *
 * In this test we build simple filtering pipeline consisting of:
 *
 *  - two random number generators ("value" and "control")
 *  - one filtering node consuming random numbers from generators, producing an
 *    which confirms propagation of "event" only if "ctrl" is greater
 *    than "value". This way we check, that "skip" return code is treated in a
 *    proper manner.
 *
 *
 *    |             Inputs              ||              Result              |
 *    | ctrl > value | count exhausted  ||  Return code   |   Exception   |
 *    +---------------------------------++------------------+---------------+
 *    |     true     |    false         ||      ok          |   no          |
 *    |     false    |    false         ||      skip        |   no          |
 *    |     true     |    true          ||      ok          |   no          |
 *    |     false    |    true          ||      error       |   no          |
 * */

# include "utest.hpp"
# include "goo_dataflow/framework.hpp"
# include "goo_dataflow/worker.hpp"

# include <random>

// Define this to avoid checking for expected recoverable exceptions and make
// the module to test only error-propagation algorithm.
# define NO_EXCEPTIONS_TESTS

namespace gdf = goo::dataflow;

static const int _static_eventSequence[] = {
    gdf::EvalStatus::ok,
    gdf::EvalStatus::ok,
    gdf::EvalStatus::ok,
    gdf::EvalStatus::ok,
    gdf::EvalStatus::skip,
    gdf::EvalStatus::skip,
    gdf::EvalStatus::done,
    gdf::EvalStatus::ok,
    gdf::EvalStatus::skip,
    gdf::EvalStatus::error,
};

// Data source emitting the "events"
class EventSource : public gdf::iProcessor {
public:
    /// Event type
    struct Type {
        int value;
        size_t id;
    };
private:
    size_t _count;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<Type>( "value", { _static_eventSequence[_count], _count} );
        ++_count;
        return 0;
    }
public:
    EventSource( int seed=1 ) : _count(0) {
        out_port<Type>("value");
    }
};

// Filter, performing the "processing" and "discrimination"
class TestingFilter : public gdf::iProcessor {
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        int res;
        auto & ev = vm.get<EventSource::Type>("v_i");
        vm.set<EventSource::Type>( "v_o", ev  );
        return ev.value;
    }
public:
    TestingFilter( ) {
        in_port<EventSource::Type>("v_i");
        out_port<EventSource::Type>("v_o");
    }
};

// End-point consumer, tracking events passed through the filters.
class Recorder : public gdf::iProcessor {
private:
    std::vector<EventSource::Type> _values;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        _values.push_back( vm.get<EventSource::Type>("value1") );
        _values.push_back( vm.get<EventSource::Type>("value2") );
        return 0;
    }
public:
    Recorder() {
        in_port<EventSource::Type>( "value1" );
        in_port<EventSource::Type>( "value2" );
    }
};

GOO_UT_BGN( DataflowErr, "Dataflow errors" ) {
    // Declare framework
    gdf::Framework fw;
    
    # ifndef NO_EXCEPTIONS_TESTS /*{{{*/
    // We expect here a recoverable error of the framework, not being able to
    // find the processor with such a name identifier (non-empty case).
    ut_Goo_EXPECT_THROW_BGN {
            fw.precedes( "one", "two", "three", "four" );
    } ut_Goo_EXPECT_THROW_CODE_END( noSuchKey,
            "Connecting of non-existing processors in empty set");
    # endif /*}}}*/

    // Declare source
    EventSource src;
    fw.impose( "src", src );

    // Declare two (concurrent) testing filters
    TestingFilter filter1, filter2;
    fw.impose( "filter#1", filter1 );
    fw.impose( "filter#2", filter2 );

    // Declare event recording processor
    Recorder rec;
    fw.impose( "recorder", rec );

    # ifndef NO_EXCEPTIONS_TESTS /* {{{ */
    // We expect here a recoverable error of the framework, not being able to
    // find the processor with such a name identifier (non-empty case #1).
    ut_Goo_EXPECT_THROW_BGN {
        fw.precedes( "one", "two", "three", "four" );
    } ut_Goo_EXPECT_THROW_CODE_END( noSuchKey, "Non-existing processor in the"
            " empty set." );
    # endif /* }}} */

    // Connect nodes
    fw.precedes( "src", "value",        "filter#1", "v_i" );
    fw.precedes( "src", "value",        "filter#2", "v_i" );
    # ifndef NO_EXCEPTIONS_TESTS /*{{{*/
    // We expect here a recoverable error of the framework, not being able to
    // find the processor with such a name identifier (non-empty case #2).
    ut_Goo_EXPECT_THROW_BGN {
        fw.precedes( "src", "value", "three", "four" );
    } ut_Goo_EXPECT_THROW_CODE_END( noSuchKey, "Non-existing processor in the"
            " non-empty set." );
    // We expect here a recoverable error of the framework, not being able to
    // find the port with such a name identifier.
    ut_Goo_EXPECT_THROW_BGN {
        fw.precedes( "filter#1", "v_z", "recorder", "value" );
    } ut_Goo_EXPECT_THROW_CODE_END( noSuchKey, "Non-existing port in the"
            " non-empty set." );
    // We expect here a recoverable error of the framework, reacting on an
    // attempt to mess with the output/input ports
    ut_Goo_EXPECT_THROW_BGN {
        fw.precedes( "filter#1", "v_i", "recorder", "value" );
    } ut_Goo_EXPECT_THROW_CODE_END( badState, "Messed up I/O ports." );
    # endif  /* }}} */
    fw.precedes( "filter#1", "v_o",     "recorder", "value1" );
    fw.precedes( "filter#2", "v_o",     "recorder", "value2" );

    //fw.generate_dot_graph(os);

    # if 1
    // Process the DAG in two concurrent threads
    std::thread          * ts[2];
    gdf::JournaledWorker * ws[2];
    for( size_t nThread = 0; nThread < 2; ++nThread ) {
        ws[nThread] = new gdf::JournaledWorker( fw );
        ts[nThread] = new std::thread( &gdf::JournaledWorker::run, ws[nThread] );
    }
    // ...
    for( size_t nThread = 0; nThread < 2; ++nThread ) {
        ts[nThread]->join();
        delete ts[nThread];
        delete ws[nThread];
    }
    # endif
} GOO_UT_END( DataflowErr, "Dataflow" )

