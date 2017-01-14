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

# ifndef H_GOO_NUMERIC_WRAPPER_H
# define H_GOO_NUMERIC_WRAPPER_H

namespace goo {
namespace mixins {

//
// Silly Numeric Wrapper
//

# define vinl virtual inline
template<typename NumericAtomicType>
struct NumericWrapper {
    NumericAtomicType number;
    /*explicit*/ operator NumericAtomicType&() { return number; }
    /*explicit*/ operator const NumericAtomicType&() const { return number; }
    NumericWrapper(const NumericAtomicType & v) : number(v) {}


    vinl bool operator!= (const NumericAtomicType& v) const {
        return number != v;
    }
    vinl bool operator== (const NumericAtomicType& o) const {
        return !(number!=o);
    }


    vinl bool operator> (const NumericAtomicType& v) const {
        return number > v;
    }
    vinl bool operator<= (const NumericAtomicType& o) const {
        return !(number > o);
    }


    vinl bool operator< (const NumericAtomicType& v) const {
        return  number < v;
    }
    vinl bool operator>= (const NumericAtomicType& o) const {
        return !(number < o);
    }


    template<typename OperandT>
    inline void operator*= ( const OperandT & t ) {
        number *= t;
    }
    template<typename OperandT>
    inline NumericAtomicType operator* ( const OperandT & o ) const {
        return o*number;
    }


    template<typename OperandT>
    inline void operator/= ( const OperandT & t ) {
        number /= t;
    }
    template<typename OperandT>
    inline NumericAtomicType operator/ ( const OperandT & o ) const {
        return number/o;
    }


    template<typename OperandT>
    inline void operator+= ( const OperandT & t ) {
        number += t;
    }
    template<typename OperandT>
    inline NumericAtomicType operator+ ( const OperandT & o ) const {
        return number+o;
    }


    template<typename OperandT>
    inline void operator-= ( const OperandT & t ) {
        number -= t;
    }
    template<typename OperandT>
    inline NumericAtomicType operator- ( const OperandT & o ) const {
        return number-o;
    }
};
# undef vinl

}  // namespace mixins
}  // namespace goo

# endif  // H_GOO_NUMERIC_WRAPPER_H

