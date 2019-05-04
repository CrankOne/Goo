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
        port<int>("value");
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
        port<int>("x1");    port<int>("x2");    port<int>("x3");
        port<int>("x4");    port<int>("x5");    port<int>("x6");

        port<int>("S");
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
        port<int>("a");    port<int>("b");

        port<int>("c");
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
        port<int>("A");    port<int>("B");
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
    # if 1
    {
        // Connect dices to 6-input sum
        fw.precedes( sum6, "x1", dices + 0, "value");
        fw.precedes( sum6, "x2", dices + 1, "value");
        fw.precedes( sum6, "x3", dices + 2, "value");
        fw.precedes( sum6, "x4", dices + 3, "value");
        fw.precedes( sum6, "x5", dices + 4, "value");
        fw.precedes( sum6, "x6", dices + 5, "value");
        // Connect dices and pairwise sums
        fw.precedes( sum2 + 0, "a", dices + 1, "value" );
        fw.precedes( sum2 + 0, "b", dices + 2, "value" );
        fw.precedes( sum2 + 1, "a", dices + 4, "value" );
        fw.precedes( sum2 + 1, "b", dices + 5, "value" );
        fw.precedes( sum2 + 2, "a", dices + 0, "value" );
        fw.precedes( sum2 + 2, "b", sum2  + 0, "c"     );
        fw.precedes( sum2 + 3, "a", dices + 3, "value" );
        fw.precedes( sum2 + 3, "b", sum2  + 1, "c"     );
        fw.precedes( sum2 + 4, "a", sum2  + 2, "c"     );
        fw.precedes( sum2 + 4, "b", sum2  + 3, "c"     );
        // Compare pairwise sum result with 6-sum
        fw.precedes( sum2 + 4, "c", cmp, "A" );
        fw.precedes( sum6    , "S", cmp, "B" );
    }
    fw._recache();
    # endif
    delete [] dices;
    delete sum6;
    delete [] sum2;
    delete cmp;
} GOO_UT_END( Dataflow, "Bitset", "DFS_DAG" )
