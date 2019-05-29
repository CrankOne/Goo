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

namespace gdf = goo::dataflow;

static const int _static_eventSequence[] = {
    1,  1,  1,  1,  /* Four ordinary events */
    2,  1,  2,  3,  /* Skip two events and abort propagation with error code */
    1,  2,  1, -1,  /* Skip one event and abort propagation with exception */
    1,  1,  1,  1,
};

// PRG
class EventSource : public gdf::iProcessor {
public:
    typedef size_t Type;
private:
    size_t _count;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<Type>( "value", _count++ );
        return 0;
    }
public:
    EventSource( int seed=1 ) : _count(0) {
        out_port<Type>("value");
    }
};

class TestingFilter : public gdf::iProcessor {
private:
    const int * _ctrlPtr;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<EventSource::Type>( "v_o"
                            , vm.get<EventSource::Type>("v_i1") + vm.get<EventSource::Type>("v_i2") );
        return *_ctrlPtr++;
    }
public:
    TestingFilter( size_t count ) : _count(count) {
        in_port<EventSource::Type>("v_i1");
        in_port<EventSource::Type>("v_i2");
        out_port<EventSource::Type>("v_o");
    }
};

class Recorder : public gdf::iProcessor {
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<Type>( "value", _g() );
        return 0;
    }
};

GOO_UT_BGN( DataflowErr, "Dataflow errors" ) {
    // Declare framework
    gdf::Framework fw;
    
    // Declare two event sources
    EventSource src1, src2;
    fw.impose( "src#1", src1 );
    fw.impose( "src#1", src2 );

    // Declare testing filter
    TestingFilter filter;
    fw.impose( "filter", filter );

    // Declare event recordin processor
    Recorder rec;
    fw.impose( "recorder" );

    // Connect nodes
    fw.precedes( "src#1", "value",       "filter", "v_i1" );
    fw.precedes( "src#2", "value",       "filter", "v_i2" );
    fw.precedes( "filter", "v_o",        "recorder", "input" );

} GOO_UT_END( DataflowErr, "Dataflow" )

