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
 * */

# include "utest.hpp"
# include "goo_dataflow/framework.hpp"
# include "goo_dataflow/worker.hpp"

# include <random>

namespace gdf = goo::dataflow;

// PRG
class Random : public gdf::iProcessor {
public:
    typedef std::mt19937::result_type Type;
private:
    std::mt19937 _g;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        vm.set<Type>( "value", _g() );
        return 0;
    }
public:
    Random( int seed=1 ) : _g(seed) {
        out_port<Type>("value");
    }
};

// Passes value "v_i" only if "ctrl" > "value" AND current event's no is lesser
// than parameter "count".
class StochasticFilter : public gdf::iProcessor {
private:
    size_t _count;
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        if( _count )
            --_count;
        auto value = vm.get<Random::Type>("v_i")
           , ctrl = vm.get<Random::Type>("ctrl")
           ;
        if( ctrl > value ) {
            if( _count )
                return gdf::EvalStatus::skip;  // block further propagation
            else
                vm.set<bool>("error", true);
        } else {
            vm.set<bool>("error", false);
        }
        vm.set<Random::Type>("v_o", value);
        return 0;
    }
public:
    StochasticFilter( size_t count ) : _count(count) {
        in_port<Random::Type>("v_i");
        in_port<Random::Type>("ctrl");

        out_port<Random::Type>("v_o");
        out_port<bool>("error");
    }
};

class Recorder : public gdf::iProcessor {
protected:
    virtual gdf::EvalStatus _V_eval( gdf::ValuesMap & vm ) override {
        auto value = vm.get<Random::Type>("v_i")
           , ctrl = vm.get<Random::Type>("ctrl")
           ;
        if( ctrl <= value ) {
            if( vm.get<bool>("error") ) {
                emraise( expected, "%u <= %u, expected."
                       , ctrl, value );
            } else {
                emraise( badState, "%u <= %u, not expected."
                       , ctrl, value );
            }
        }
        return 0;
    }
public:
    Recorder() {
        in_port<Random::Type>("v_i");
        in_port<Random::Type>("ctrl");
        in_port<bool>("error");
    }
};

GOO_UT_BGN( DataflowErr, "Dataflow errors" ) {
    gdf::Framework fw;
    // ...    
} GOO_UT_END( DataflowErr, "Dataflow" )

