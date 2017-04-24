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

/** \addtogroup utVCtr
 *  @{
 */

/**@file vcopy.cpp
 * @brief Goo's virtual ctr testing unit.
 * @note Some functions of this testing module only available in
 * debug build type (i.e. when macro NDEBUG not defined).
 * */

# include "utest.hpp"
# include "goo_vcopy.tcc"

# include <climits>

/// Some base class playing role of base
/// for virtual copying. This one has no pure-virtual
/// methods, so it may be reasonable to force its
/// cloning method to be implemented.
class Base : public goo::mixins::iDuplicable<Base, Base, Base, true> {
public:
    static UShort new_id() {
        return USHRT_MAX*(double(rand())/RAND_MAX);
    }
private:
    const UShort _id,
                 _copiedFromID
                 ;
public:
    Base() : _id(new_id()), _copiedFromID(0) {}
    Base( UShort id_ ) : _id(id_), _copiedFromID(0) {}
    Base( const Base & orig ) : _id(new_id()), _copiedFromID( orig.id() ) {}
    virtual ~Base() {}
    UShort id() const { return _id; }
    UShort original_id() const { return _copiedFromID; }
};  // class Base

class AbstractBase : public goo::mixins::iDuplicable<AbstractBase, AbstractBase, AbstractBase, false> {
public:
    static UShort new_id() {
        return USHRT_MAX*(double(rand())/RAND_MAX);
    }
private:
    const UShort _id,
                 _copiedFromID
                 ;
public:
    AbstractBase() : _id(new_id()), _copiedFromID(0) {}
    AbstractBase( UShort id_ ) : _id(id_), _copiedFromID(0) {}
    AbstractBase( const AbstractBase & orig ) : _id(new_id()), _copiedFromID( orig.id() ) {}
    virtual ~AbstractBase() {}
    UShort id() const { return _id; }
    UShort original_id() const { return _copiedFromID; }
};  // class AbstractBase

/// Direct duplicable descendant of duplicable base.
template<typename BaseT>
class Derived1 : public goo::mixins::iDuplicable<BaseT, Derived1<BaseT>, BaseT> {
    typedef typename goo::mixins::iDuplicable<BaseT, Derived1<BaseT>, BaseT>::DuplicableParent DuplicableParent;
public:
    Derived1() : DuplicableParent( this->new_id() ) {}
    Derived1( const Derived1<BaseT> & copy ) : DuplicableParent( copy ) {}
};  // Derived1

/// Far abstract descendant of duplicable base.
template<typename BaseT>
class Derived2 : public BaseT {
protected:
    virtual void _V_some_interfacing_function() = 0;
public:
    Derived2() {}
    Derived2( const Derived2 & copy ) : BaseT( copy ) {}
};  // Derived2

/// Far duplicable descendant of duplicable base.
template<typename BaseT>
class Derived3 : public goo::mixins::iDuplicable<BaseT, Derived3<BaseT>, Derived2<BaseT>> {
    typedef typename goo::mixins::iDuplicable<BaseT, Derived3<BaseT>, Derived2<BaseT>>::DuplicableParent DuplicableParent;
protected:
    virtual void _V_some_interfacing_function() override {}
public:
    Derived3() {}
    Derived3( const Derived3<BaseT> & orig ) : DuplicableParent(orig) {}
};  // class Derived3

/// Bad duplicable class.
template<typename BaseT>
class BadDerived : public /*goo::mixins::iDuplicable<BaseT, BadDerived, Derived3>*/
                          Derived3<BaseT> {
public:
    BadDerived( const BadDerived & ) = delete;
    BadDerived() {}
};  // class Derived3


template<typename BaseT> void
run_test_on_base( std::ostream & os ) {
    {   // Direct descendant copy check:
        Derived1<BaseT> a;
        const BaseT * baseA = &a;
        os << "  Checking copy of Derived1 <- Base ... ";
        BaseT * copyAPtr = goo::clone( baseA );
        _ASSERT( copyAPtr, "Cloning routine produced a null pointer." );
        _ASSERT( copyAPtr->id() != baseA->id(),
                 "IDs of copy and original matches." );
        _ASSERT( copyAPtr->original_id() == baseA->id(),
                 "Wrong original ID set for copy." )
        os << "ok (expected)" << std::endl;
    }
    {   // Far descendant copy check:
        Derived3<BaseT> a;
        const BaseT * baseA = &a;
        os << "  Checking copy of Derived3 <- Derived2 <- Base ... ";
        BaseT * copyAPtr = goo::clone( baseA );
        _ASSERT( copyAPtr, "Cloning routine produced a null pointer." );
        _ASSERT( copyAPtr->id() != baseA->id(),
                 "IDs of copy and original matches." );
        _ASSERT( copyAPtr->original_id() == baseA->id(),
                 "Wrong original ID set for copy." )
        os << "ok (expected)" << std::endl;
    }

    # ifndef NDEBUG
    {   // Bad derived (unimplemented copy ctr) check:
        BadDerived<BaseT> a;
        bool hadFailure = false;
        const BaseT * baseA = &a;
        os << "  Checking copy of BadDerived <- Derived2 <- Base ... ";
        try {
            BaseT * copyAPtr = goo::clone( baseA );
            (void)(copyAPtr);
            delete copyAPtr;
        } catch( goo::Exception & e ) {
            if( goo::Exception::dbgBadArchitect == e.code() ) {
                hadFailure = true;
            } else {
                throw;
            }
        }
        _ASSERT( hadFailure, "RTTI check for correct copy ctr invokation "
                 "didn't detect an error.");
        os << "failure (expected)" << std::endl;
    }
    # endif
}

GOO_UT_BGN( VCtr, "Virtual copy constructor" ) {
    os << "Checking non-virtual base copying ";
    {   // Non-abstract base copy production (does not much sense,
        // however it is desirable for some design aspects):
        Base a;
        Base * aCopyPtr = goo::clone( &a );
        _ASSERT( aCopyPtr,
                 "Base copy: cloning routine produced a null pointer." );
        _ASSERT( aCopyPtr->id() != a.id(),
                 "Base copy: IDs of copy and original matches." );
        _ASSERT( aCopyPtr->original_id() == a.id(),
                 "Base copy: wrong original ID set for copy." )
        delete aCopyPtr;
    }
    os << "(base itself cloned ok):" << std::endl;
    run_test_on_base<Base>( os );
    os << "Checking abstract base copying:" << std::endl;
    run_test_on_base<AbstractBase>( os );
} GOO_UT_END( VCtr )

/** @} */

