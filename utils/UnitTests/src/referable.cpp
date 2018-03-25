/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

// Created by crank on 25.03.18.

# include "utest.hpp"
# include "goo_dict/value.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

// -------------------------------------------------------------------------- //

/// A simple value keeping referable class.
template<typename T>
struct SimpleValue {
    T value;

    SimpleValue(T v) : value(v) {}
    operator T&() { return value; }
};

// -------------------------------------------------------------------------- //

template<typename T> struct MockMutator;  // fwd

/// Example value-referencing referable object class, uses dedicated mutator
/// class to perform value changing/assignment operations.
template<typename T>
class ByAddrReference {
private:
    T * _value;
    const char * _descr;
protected:
    ByAddrReference() : _value(nullptr), _descr("") {}
    void value( const T & v ) { *_value = v; }
public:
    ByAddrReference( T* vPtr, const char * d ) : _value(vPtr), _descr(d) {}

    const T & value() const { return *_value; }
    const T * value_addr() const { return _value; }
    const char * description() const { return _descr; }
    operator const T &() const { return *_value; }

    template<typename> friend struct MockMutator;
};

/// Example mutator class, catching assignment.
template<typename T>
struct MockMutator {
    static std::ostream * os;
    static const T * lValAddress;
    ByAddrReference<T> & target;
    MockMutator( ByAddrReference<T> & t ) : target(t) {}
    MockMutator & operator=( T newVal ) {
        if( os ) {
            *os << "Setting addr " << (const void *) &(target.value())
                << " to value " << newVal
                << " (" << target.description() << ")."
                << std::endl;
        }
        target.value( newVal );
        lValAddress = target.value_addr();
        return *this;
    }
    // here optional:
    // T & operator() ...
};

template<typename T> std::ostream * MockMutator<T>::os = nullptr;
template<typename T> const T * MockMutator<T>::lValAddress = nullptr;

// -------------------------------------------------------------------------- //

GOO_UT_BGN( Referable, "goo::dict referable helpers" ) {
    {  // Simple value aggregators
        goo::dict::ReferableTraits<SimpleValue>::ReferableWrapper<int> v(2);
        v.as<int &>() = 3.14;
        float iv = v.as<float>();
        _ASSERT(iv == 3
                , "Unexpected type cast. v.as<float>() returned %f instead of 3."
                , iv);

        # ifdef _Goo_m_SHALL_FAIL
        // type mismatch, float reference requested from int object
        v.as<float &>() = 1.2;
        # endif
    }

    {  // Referencing referable objects (memblock mappings)
        char * layout = new char[4 * sizeof(float)];
        bzero(layout, 4 * sizeof(float));

        goo::dict::ReferableTraits<ByAddrReference>
                 ::ReferableWrapper<float> v0(((float *) layout), "one")
                                         , v1(((float *) layout) + 1, "two")
                                         , v2(((float *) layout) + 2, "three")
                                         , v3(((float *) layout), "alias to one")
                                         ;

        # ifdef _Goo_m_SHALL_FAIL
        // shall not compile (must use mutator)
        v1.as<float &>() = 1.;
        # endif

        v0.as<MockMutator<float>>() = 1.23;
        _ASSERT((const float *) layout == MockMutator<float>::lValAddress
                , "Wrong reference: %p instead of %p"
                , MockMutator<float>::lValAddress
                , layout);
        v1.as<MockMutator<float>>() = 3.21;
        v2.as<MockMutator<float>>() = 4.56;
        float iv = v0.as<float>();
        _ASSERT(fabsf((float) 1.23 - iv) < 1e-6
                , "Straightforward value setting failure: %f != 1.23", iv);
        iv = v1.as<float>();
        _ASSERT(fabsf((float) 3.21 - iv) < 1e-6
                , "Straightforward value setting failure: %f != 3.21", iv);
        _ASSERT(fabsf(v0.as<float>() - v3.as<float>()) < 1e-6
                , "Aliasing reference test failure.")

        delete[] layout;
    }
} GOO_UT_END( Referable )

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)


