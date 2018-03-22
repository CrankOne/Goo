# ifndef H_GOO_ITERABLE_MIXIN_H
# define H_GOO_ITERABLE_MIXIN_H

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

# include "goo_config.h"
# include "goo_sys/exception.hpp"
# include "goo_mixins/basicOperators.tcc"
# include <iterator>

namespace goo {

/* TODO: make a Doxygen section from this note
 * Iterators
 *
 * The goo iterators are nothing but merely lexical helpers to automate the
 * most frequent iterators definitions in C++.
 *
 * Direction:
 *  - BaseForwardIterator<FinalT, SymT>
 *  - BaseBidirIterator<FinalT, SymT>
 * Access:
 *  - BaseOutputIterator<FinalT, SymT, T>
 *  - BaseInputIterator<FinalT, SymT, T>
 *  - BaseIOIterator<FinalT, SymT, T>
 * Comparison:
 *  - BaseComparableIterator<FinalT, SymT>
 *  - DirectionComparableIterator<FinalT, SymT>
 */

namespace iterators {

/**@brief Basic comparison trait of iterators.
 *
 * Defines how iterator may be compared. Overloads the '==' and '!=' operators.
 **/
template<typename FinalT, typename SymT>
struct BaseComparableIterator {
    bool operator!= (const FinalT & eit) const {
        return static_cast<const FinalT*>(this)->sym() != eit.sym();
    }

    bool operator== (const FinalT & o) const {
        return !(*this != o);
    }
};


/**@brief Iterator ability for increment itself with pre/postfix operator.
 *
 * Overloads 'a++' and '++a' operators.
 */
template<typename FinalT,
         typename SymT>
struct BaseForwardIterator {
    FinalT & operator++() {
        ++(static_cast<FinalT*>(this)->sym());
        return *static_cast<FinalT*>(this);
    }

    FinalT operator++ (int) {
        return FinalT( ++ (static_cast<FinalT &>(*this)) );
    }
};  // BaseForwardIterator



template<typename FinalT, typename SymT, typename T>
struct BaseOutputIterator {
    T & operator*() const {
        return *(static_cast<const FinalT*>(this)->sym());
    }
};  // BaseOutputIterator



template<typename FinalT, typename SymT, typename T>
struct BaseInputIterator {
    static_assert(!std::is_const<T>::value,
            "Input iterator can not be dereferenced for const value type." );
    T & operator*() {
        return *(static_cast<FinalT*>(this)->sym());
    }
};  // BaseOutputIterator


template<typename FinalT, typename SymT, typename T>
struct BaseIOIterator {
    const T & operator*() const {
        return *(static_cast<const FinalT*>(this)->sym());
    }

    T & operator*() {
        return *(static_cast<FinalT*>(this)->sym());
    }
};


template<typename FinalT, typename SymT>
struct BaseBidirIterator : public BaseForwardIterator<FinalT, SymT> {
    typedef BaseForwardIterator<FinalT, SymT> Parent;

    FinalT & operator--() {
        --(static_cast<FinalT*>(this)->sym());
        return static_cast<FinalT &>(*this);
    }
    FinalT operator-- (int) {
        return FinalT( -- (static_cast<FinalT &>(*this)) );
    }
};  // BaseForwardIterator


template<typename FinalT,
         typename SymT>
struct DirectionComparableIterator :
                            public BaseComparableIterator<FinalT, SymT> {
    typedef DirectionComparableIterator<FinalT, SymT> Self;
    typedef BaseComparableIterator<FinalT, SymT> Parent;

    bool operator>( const FinalT & prhs ) const {
        return static_cast<const FinalT*>(this)->sym() > prhs.sym();
    }

    bool operator<  (const FinalT & o) const {
        return (  static_cast<const FinalT*>(this)->sym() != o.sym
            && !( static_cast<const FinalT*>(this)->sym()  > o.sym ) );
    }
};  // DirectionComparableIterator



template<
        template<class, class> class DirectionTT,
        template<class, class, class> class AccessTT,
        template<class, class> class ComparisonTT,
        typename FinalT,
        typename SymT,
        typename T,
        typename DistanceT=void>
struct Iterator : public DirectionTT<FinalT, SymT>,
                 public AccessTT<FinalT, SymT, T>,
                 public ComparisonTT<FinalT, SymT> {
    typedef DirectionTT<FinalT, SymT> Direction;
    typedef AccessTT<FinalT, SymT, T> Access;
    typedef ComparisonTT<FinalT, SymT> Comparison;
};


template<template<class, class, class> class AccessTT,
         typename FinalT,
         typename SymT,
         typename T,
         typename DistanceT>
struct Iterator<BaseBidirIterator,
               AccessTT,
               DirectionComparableIterator,
            FinalT,
            SymT,
            T,
            DistanceT> : public BaseBidirIterator<FinalT, SymT>,
                         public AccessTT<FinalT, SymT, T>,
                         public DirectionComparableIterator<FinalT, SymT> {
    typedef BaseBidirIterator<FinalT, SymT> Direction;
    typedef AccessTT<FinalT, SymT, T> Access;
    typedef DirectionComparableIterator<FinalT, SymT> Comparison;

    typedef Iterator<BaseBidirIterator, AccessTT, DirectionComparableIterator,
                     FinalT, SymT, T, DistanceT> Self;

    FinalT operator+( const DistanceT & o ) const {
        FinalT C(*static_cast<const FinalT*>(this));
        return C += o;
    }

    FinalT & operator+= ( const DistanceT & d ) {
        static_cast<FinalT*>(this)->sym() += d;
        return *static_cast<FinalT*>(this);
    }

    FinalT operator-( const DistanceT & o ) const {
        FinalT C(*static_cast<const FinalT*>(this));
        return C -= o;
    }

    FinalT & operator-= ( const DistanceT & d ) {
        static_cast<FinalT*>(this)->sym() += -d;
        return *static_cast<FinalT*>(this);
    }

    DistanceT operator- ( const FinalT & prhs ) const {
        return static_cast<const FinalT*>(this)->sym() - prhs.sym();
    }
};

}  // namespace ::goo::iterators

namespace mixins {

//template
//class Iterable

}  // namespace ::goo::mixins

# if 0
// XXX:

template<   typename IteratorSymT,
            typename DereferencedTypeT,
            typename EndIteratorClassT>
class ConstOutputIteratorBase : public mixins::PostfixIncOp<EndIteratorClassT> {
public:
    typedef EndIteratorClassT EndIteratorClass;
    typedef IteratorSymT IteratorSym;
    typedef DereferencedTypeT DereferencedType;
protected:
    IteratorSym _sym;

    virtual DereferencedType & _V_dereference_sym( const IteratorSym & ) const = 0;
public:
    ConstOutputIteratorBase() {}
    ConstOutputIteratorBase( const IteratorSym & itSym ) : _sym(itSym) {}
    virtual~ConstOutputIteratorBase() {}

    const DereferencedType & operator*() const {
        return _V_dereference_sym( _sym );
    }
};

template<   typename IteratorSymT,
            typename DereferencedTypeT,
            typename EndIteratorClassT>
class OutputIteratorBase : public ConstOutputIteratorBase<  IteratorSymT,
                                                            DereferencedTypeT,
                                                            EndIteratorClassT> {
public:
    typedef EndIteratorClassT EndIteratorClass;
    typedef IteratorSymT IteratorSym;
    typedef DereferencedTypeT DereferencedType;
public:
    DereferencedType & operator*() const {
        return _V_dereference_sym( ConstOutputIteratorBase<
                    IteratorSym, DereferencedType, EndIteratorClass>::_sym );
    }
};  // class OutputIteratorBase

namespace mixins {

/**@class Iterable
 * @brief iterable container mixin.
 *
 * Conception of iterator is a common thing. See:
 * https://en.wikipedia.org/wiki/Iterator
 *
 * This implementation needs to be immersed in container implementation.
 * In most base it must provide a method for increment operator.
 *
 * In order to keep compliance with stdlib, some additional method must be
 * provided by iterator implementation (see nested Iterable::Iterator help
 * for details).
 */
template<typename DereferencedTypeT,
         typename IteratorSymT>
class IterableBase {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;
    typedef IterableBase<DereferencedType, IteratorSym> Self;
protected:
    template<typename EndIteratorClassT>  /// Keep 'Iterator' name for default.
    class ConstIteratorBase : public PostfixIncOp<EndIteratorClassT> {
    public:
        typedef Self Authority;
        typedef EndIteratorClassT EndIteratorClass;
    protected:
        Authority * _authorityPtr;  ///< no check for authority for performance.
        IteratorSym _sym;
        void _check_authority_is_set() const { DBG_NULLPTR_CHECK( _authorityPtr, "Empty iterator %p.", this ); }
        // Ctr/dtr --- protected from explicit use.
        ConstIteratorBase() : _authorityPtr(nullptr) {}
    public:
        ConstIteratorBase( const IteratorSym & d ) : 
                _authorityPtr(nullptr), _sym(d) {}

        // authority acquizition:
        const Authority & authority() const {
                _check_authority_is_set();
                return *_authorityPtr; }

        // increment:
        virtual EndIteratorClass & operator++() override {
            _check_authority_is_set();
            _authorityPtr->_V_increase_iterator_sym( _sym );
        }
    };

    template<typename EndIteratorClassT>  /// Keep 'Iterator' name for default.
    class IteratorBase : public ConstIteratorBase<EndIteratorClassT> {
    public:
        IteratorBase( const IteratorSym & d ) : 
                ConstIteratorBase<EndIteratorClassT>(d) {}

        // authority acquizition:
        typename ConstIteratorBase<EndIteratorClassT>::Authority & authority() {
                ConstIteratorBase<EndIteratorClassT>::_check_authority_is_set();
                return *ConstIteratorBase<EndIteratorClassT>::_authorityPtr; }
    };
protected:
    virtual void _V_increase_iterator_sym( IteratorSym & ) = 0;
};  // class IterableBase

//
// OUTPUT ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT>
class OutputIterable : public virtual IterableBase<DereferencedTypeT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;
    typedef IterableBase<DereferencedType,
                         IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class OutputIterator : public virtual Parent::template IteratorBase<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template IteratorBase<EndIteratorClassT> ParentIterator;

        DereferencedType & operator*() {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_dereference_iterator_sym( ParentIterator::_sym );
        }
    };

    virtual DereferencedType & _V_dereference_iterator_sym( IteratorSym & ) = 0;
};

//
// COMPARABLE ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT>
class ComparableIterable : public virtual IterableBase<DereferencedTypeT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;

    typedef IterableBase<DereferencedType,
                         IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class ComparableIterator : public IdentityOp<EndIteratorClassT>,
                               public virtual Parent::template ConstIteratorBase<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ConstIteratorBase<EndIteratorClassT> ParentIterator;

        virtual bool operator!= (const EndIteratorClass & eit) const override {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_compare_iterators(
                        ParentIterator::_sym,
                        eit._sym );
        }
    };

    virtual void _V_compare_iterators( const IteratorSym &, const IteratorSym & ) = 0;
};

//
// BIDIRECTIONAL ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT>
class BidirIterable : public virtual ComparableIterable<DereferencedTypeT,
                                                   IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;

    typedef ComparableIterable<DereferencedType,
                          IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class BidirIterator : public virtual Parent::template ComparableIterator<EndIteratorClassT>,
                          public virtual PostfixDecOp<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ComparableIterator<EndIteratorClassT> ParentIterator;
        virtual EndIteratorClass & operator--() override {
            ParentIterator::_check_authority_is_set();
            ParentIterator::_authorityPtr->_V_decrease_iterator_sym( ParentIterator::_sym );
        }
    };

    virtual void _V_decrease_iterator_sym( IteratorSym & ) = 0;
};

//
// INPUT ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT>
class InputIterable : public virtual ComparableIterable<DereferencedTypeT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;

    typedef ComparableIterable<DereferencedType,
                          IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class InputIterator : public virtual Parent::template ComparableIterator<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ComparableIterator<EndIteratorClassT> ParentIterator;

        const DereferencedType & operator*() const {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_dereference_iterator_sym( ParentIterator::_sym );
        }
    };

    virtual const DereferencedType & _V_dereference_iterator_sym( const IteratorSym & ) const = 0;
};

//
// FORWARD ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT>
class ForwardIterable : public virtual ComparableIterable<DereferencedTypeT,
                                                     IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;

    typedef ComparableIterable<DereferencedType,
                          IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class ForwardIterator : public virtual Parent::template ComparableIterator<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ComparableIterator<EndIteratorClassT> ParentIterator;

        ForwardIterator(){}
    };
};

//
// RANDOM ACCESS ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorSymT,
         typename OffsetT>
class RandomIterable : public virtual BidirIterable<DereferencedTypeT,
                                                     IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorSymT IteratorSym;
    typedef OffsetT Offset;

    typedef BidirIterable<DereferencedType,
                          IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class RandomIterator : public virtual Parent::template BidirIterator<EndIteratorClassT>,
                           public virtual FullComparableOp<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ComparableIterator<EndIteratorClassT> ParentIterator;

        virtual bool operator>(const EndIteratorClass & prhs) const override {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_is_greater(
                            ParentIterator::_sym,
                            prhs );
        }

        virtual EndIteratorClass & operator+= ( const Offset & nt ) {
            ParentIterator::_check_authority_is_set();
            ParentIterator::_authorityPtr->_V_itsym_increase( ParentIterator::_sym, nt );
            return *this;
        }

        virtual EndIteratorClass & operator-= ( const Offset & nt ) {
            ParentIterator::_check_authority_is_set();
            ParentIterator::_authorityPtr->_V_itsym_decrease( ParentIterator::_sym, nt );
            return *this;
        }

        virtual DereferencedType & operator[](const Offset & nt) {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_itsym_nth_offset( ParentIterator::_sym, nt );
        }

        virtual const DereferencedType & operator[](const Offset & nt) const {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_itsym_nth_offset( ParentIterator::_sym, nt );
        }
    };
    
    virtual bool _V_is_greater( const IteratorSym &, const IteratorSym & ) = 0;
    virtual void _V_itsym_increase( IteratorSym &, const Offset & ) = 0;
    virtual void _V_itsym_decrease( IteratorSym &, const Offset & ) = 0;
    virtual DereferencedType & _V_itsym_nth_offset( const IteratorSym &, const Offset & ) = 0;
    virtual const DereferencedType & _V_itsym_nth_offset( const IteratorSym &, const Offset & ) const = 0;
};


//
//
//

enum IteratorFeatures {
    reading         = 0x1 | 0x4,
    writing         = 0x2,
    comparison      = 0x4,
    decrement       = 0x8  | 0x4,
    forward         = 0xf1 | 0x4,
    randomAccess    = 0xf2 | 0x4 | 0x8
};

}  // namespace mixins

# endif

}  // namespace goo

# endif  // H_GOO_ITERABLE_MIXIN_H

