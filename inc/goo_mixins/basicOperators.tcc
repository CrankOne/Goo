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

# ifndef H_GOO_MIXINS_H
# define H_GOO_MIXINS_H

# include <algorithm>
# include <thread>
# include <mutex>
# include <map>
# include <condition_variable>
# include <iostream>
# include <queue>
# include <sstream>
# include "goo_types.h"
# include "goo_exception.hpp"

# ifndef DNDEBUG
# define auth_cast dynamic_cast
# else
# define auth_cast static_cast
# endif

namespace goo {
namespace mixins {

//
// Stringification operator
//

struct ToOStreamOp {
protected:
    virtual void _V_to_ostream( std::ostream & os ) const = 0;
public:
    inline void to_ostream( std::ostream & os ) const {
        _V_to_ostream( os );
    }
    inline std::string to_str() const {
        std::stringstream ss;
        to_ostream(ss);
        return ss.str();
    }
};

inline std::ostream & operator<<(std::ostream& os, const ToOStreamOp & t) {
    t.to_ostream(os);
    return os;
}

# define decl_mixin_struct  template<typename SelfT> struct
# define vinl virtual inline
# define _vPub virtual public

/** @brief identity operator `==` mixin (uses `!=`)
 *
 * Curiously recurring template pattern implementing «is-equal» operator when
 * «is‐non‐equal» defined.
 */ decl_mixin_struct
IdentityOp {
private:
    mutable const SelfT * this_;
public:
    IdentityOp() : this_(0) {}
    vinl bool operator!= (const SelfT&) const = 0;
    vinl bool operator== (const SelfT& o) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        return !((*this_)!=o);
    }
};

/** @brief lesser or equals operator `<=` (uses `>`)
 *
 * Curiously recurring template pattern implementing «lesser-or-equals»
 * operator when «is-greater-than» defined.
 */ decl_mixin_struct
LesserEqOp {
private:
    mutable const SelfT * this_;
public:
    LesserEqOp() : this_(0) {}
    vinl bool operator> (const SelfT& ) const = 0;
    vinl bool operator<= (const SelfT& o) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        return !((*this_) > o);
    }
};

/** @brief greater or equals operator `>=` mixin (uses `<`)
 *
 * Curiously recurring template pattern.
 */ decl_mixin_struct
GreaterEqOp {
private:
    mutable const SelfT * this_;
public:
    GreaterEqOp() : this_(0) {}
    vinl bool operator< (const SelfT&) const = 0;
    vinl bool operator>= (const SelfT& o) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        return !((*this_) < o);
    }
};

/** @brief full set of comparison operators
 *
 * Curiously recurring template pattern,
 * uses (superposition):  `!=`, `>`  op-rs
 */ decl_mixin_struct
FullComparableOp : _vPub IdentityOp<SelfT>,
                   _vPub GreaterEqOp<SelfT>,
                   _vPub LesserEqOp<SelfT> {
private:
    mutable const SelfT * this_;
public:
    FullComparableOp() : this_(0) {}
    vinl bool operator!= (const SelfT&) const = 0;
    vinl bool operator>  (const SelfT&) const = 0;
    vinl bool operator<  (const SelfT& o) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        return ( *this_ != o && !(*this_ > o) );
    }
};

/**@brief implements postfix single increment op-r (uses prefix)
 *
 * Curiously recurring template pattern.
 */ decl_mixin_struct
PostfixIncOp {
private:
    SelfT * this_;
public:
    PostfixIncOp() : this_(0) {}
    vinl SelfT& operator++ () = 0; // prefix
    vinl SelfT  operator++ (int) {
        if(!this_){ this_ = dynamic_cast<SelfT*>(this); }
        return SelfT( ++ (*this_) );
    }
};

/** @brief implements postfix single decrement op-r (uses prefix)
 *
 * Curiously recurring template pattern.
 */decl_mixin_struct
PostfixDecOp {
private:
    SelfT * this_;
public:
    PostfixDecOp() : this_(0) {}
    vinl SelfT& operator-- () = 0;
    vinl SelfT  operator-- (int) { // prefix
        if(!this_){this_ = dynamic_cast<SelfT*>(this);}
        return SelfT( -- (*this_) );
    }
};


//
// Binary operators
//


template<typename SelfT, typename OperandT> struct
AsteriskOp {
private:
    mutable const SelfT * this_;
public:
    AsteriskOp() : this_(0) {}
    vinl void operator*= ( const OperandT & t ) = 0;
    vinl SelfT operator* ( const OperandT & o ) {
        if(!this_){this_ = dynamic_cast<const SelfT*>(this);}
        SelfT C(*this_);
        C *= o;
        return C;
    }
    vinl SelfT operator* ( const OperandT & o ) const {
        if(!this_){this_ = dynamic_cast<const SelfT*>(this);}
        SelfT C(*this_);
        C *= o;
        return C;
    }
};

// TODO
# if 0
template<typename SelfT, typename OperandT> struct
CommutativeAsteriskOp : public AsteriskOp<SelfT, OperandT> {};

template<typename SelfT, typename OperandT>
SelfT operator*( const OperandT & r,
                 const CommutativeAsteriskOp<SelfT, OperandT> & l) {
    return l.operator*(r); }
# endif

template<typename SelfT, typename OperandT> struct
SlashOp {
private:
    mutable const SelfT * this_;
public:
    SlashOp() : this_(0) {}

    vinl void operator/= ( const OperandT & t ) = 0;
    vinl SelfT operator/ ( const OperandT & o ) {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C /= o;
        return C;
    }
    vinl SelfT operator/ ( const OperandT & o ) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C /= o;
        return C;
    }
};

template<typename SelfT, typename OperandT> struct
PlusOp {
private:
    mutable const SelfT * this_;
public:
    PlusOp() : this_(0) {}

    vinl void operator+= ( const OperandT & t ) = 0;
    vinl SelfT operator+ ( const OperandT & o ) {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C += o;
        return C;
    }
    vinl SelfT operator+ ( const OperandT & o ) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C += o;
        return C;
    }
};

template<typename SelfT, typename OperandT> struct
DashOp {
private:
    mutable const SelfT * this_;
public:
    DashOp() : this_(0) {}

    vinl void operator-= ( const OperandT & t ) = 0;
    vinl SelfT operator- ( const OperandT & o ) {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C -= o;
        return C;
    }
    vinl SelfT operator- ( const OperandT & o ) const {
        if(!this_){ this_ = dynamic_cast<const SelfT*>(this); }
        SelfT C(*this_);
        C -= o;
        return C;
    }
};

template<typename Origin> struct
ImplicitUpcast {
private:
    Origin * this_;
public:
    ImplicitUpcast() : this_(dynamic_cast<Origin*>(this)) {}

    operator Origin&() {
        return *this_;
    }
    operator const Origin&() const {
        return *this_;
    }
};

}  // namespace mixins
}  // namespace goo

# undef decl_mixin_struct
# undef vinl
# undef _vPub

# endif  // H_GOO_MIXINS_H

