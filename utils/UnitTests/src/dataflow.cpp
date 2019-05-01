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
        slot<int>("value");
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
        slot<int>("x1");    slot<int>("x2");    slot<int>("x3");
        slot<int>("x4");    slot<int>("x5");    slot<int>("x6");

        slot<int>("S");
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
        slot<int>("a");    slot<int>("b");

        slot<int>("c");
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
        slot<int>("A");    slot<int>("B");
    }

    size_t total() const { return _match + _mismatch; }
    size_t n_match() const { return _match; }
    size_t n_mismatch() const { return _match; }
};

GOO_UT_BGN( Dataflow, "Dataflow framework" ) {
    gdf::Framework fw;
    auto dices = new Dice [6];
    auto sum6 = new Sum6;
    auto sum2 = new Sum2 [5];
    auto cmp = new Compare();
    {
        // Connect dices to 6-input sum
        fw.precedes( sum6, dices + 0 ).maps("value", "x1");
        fw.precedes( sum6, dices + 1 ).maps("value", "x2");
        fw.precedes( sum6, dices + 2 ).maps("value", "x3");
        fw.precedes( sum6, dices + 3 ).maps("value", "x4");
        fw.precedes( sum6, dices + 4 ).maps("value", "x5");
        fw.precedes( sum6, dices + 5 ).maps("value", "x6");
        // Connect dices and pairwise sums
        fw.precedes( sum2 + 0, dices + 1 ).maps("value", "a");
        fw.precedes( sum2 + 0, dices + 2 ).maps("value", "b");
        fw.precedes( sum2 + 1, dices + 4 ).maps("value", "a");
        fw.precedes( sum2 + 1, dices + 5 ).maps("value", "b");
        fw.precedes( sum2 + 2, dices + 0 ).maps("value", "a");
        fw.precedes( sum2 + 2, sum2  + 0 ).maps("c",     "b");
        fw.precedes( sum2 + 3, dices + 3 ).maps("value", "a");
        fw.precedes( sum2 + 3, sum2  + 1 ).maps("c",     "b");
        fw.precedes( sum2 + 4, sum2  + 2 ).maps("c",     "a");
        fw.precedes( sum2 + 4, sum2  + 3 ).maps("c",     "b");
        // Compare pairwise sum result with 6-sum
        fw.precedes( sum2 + 4, cmp ).maps("c", "A");
        fw.precedes( sum6    , cmp ).maps("S", "B");
    }
    delete [] dices;
    delete sum6;
    delete [] sum2;
    delete cmp;
} GOO_UT_END( Dataflow, "Bitset", "DFS_DAG" )
