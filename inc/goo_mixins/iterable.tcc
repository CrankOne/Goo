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
# include "goo_exception.hpp"
# include "goo_mixins/basicOperators.tcc"
# include <iterator>

namespace goo {

namespace iterators {

/**@brief An iterator abstract base.
 *
 * This template class declares common virtual base for descendant iterator
 * classes. The most basic traits of all subsequent iterator classes is to
 * keep associated state referencing value here names "iterator symbol".
 **/
template<typename SymT>
class BaseIterator {
public:
    typedef SymT Sym;
private:
    Sym _sym;
protected:
    Sym & sym() { return _sym; }
    const Sym & sym() const { return _sym; }
public:
    BaseIterator() {}
    BaseIterator( const Sym & sym ) : _sym(sym) {}
    virtual ~BaseIterator() {}
};  // BaseIterator


/**@brief Basic comparison trait of iterators.
 *
 * Defines how iterator may be compared. Overloads the '==' and '!=' operators.
 **/
template<typename FinalT,
         typename SymT>
class BaseComparableIterator : public virtual BaseIterator<SymT>,
                               public virtual mixins::IdentityOp<FinalT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef BaseIterator<Sym> Parent;
protected:
    /// Shall return true if own state and state given as an argument are
    /// equivalent.
    virtual bool _V_compare( const Sym & ) const = 0;
public:
    BaseComparableIterator() {}
    BaseComparableIterator( const SymT & s ) : Parent(s) {}
    virtual ~BaseComparableIterator() {}

    virtual bool operator!= (const Final & eit) const override {
        return _V_compare( eit.sym() );
    }
};


/**@brief Iterator ability for increment itself with pre/postfix operator.
 *
 * Overloads 'a++' and '++a' operators.
 */
template<typename FinalT,
         typename SymT>
class BaseForwardIterator : public virtual BaseIterator<SymT>,
                            public mixins::PostfixIncOp<FinalT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef BaseIterator<Sym> Parent;
protected:
    /// Shall increment iterator state.
    virtual void _V_increment( ) = 0;
public:
    BaseForwardIterator() {}
    BaseForwardIterator( const Sym & s ) : Parent(s) {}
    virtual ~BaseForwardIterator() {}

    virtual Final & operator++() override {
        _V_increment( );
        return static_cast<Final &>(*this);
    }
};  // BaseForwardIterator



template<typename FinalT,
         typename SymT,
         typename T>
class BaseOutputIterator : public virtual BaseIterator<SymT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef T Value;
    typedef BaseIterator<SymT> Parent;
protected:
    /// Shall return immutable reference to value referenced by the given state.
    virtual const Value & _V_dereference( const Sym & ) const = 0;
public:
    BaseOutputIterator() {}
    BaseOutputIterator( const SymT & s ) : Parent(s) {}
    virtual ~BaseOutputIterator() {}

    const Value & operator*() const {
        return _V_dereference( Parent::sym() );
    }
};  // BaseOutputIterator



template<typename FinalT,
         typename SymT,
         typename T>
class BaseInputIterator : public virtual BaseIterator<SymT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef T Value;
    typedef BaseIterator<SymT> Parent;
protected:
    /// Shall return mutable reference to value referenced by the given state.
    virtual Value & _V_dereference_mutable( Sym & ) = 0;
public:
    BaseInputIterator() {}
    BaseInputIterator( const SymT & s ) : Parent(s) {}
    virtual ~BaseInputIterator() {}

    Value & operator*() {
        return _V_dereference_mutable( Parent::sym() );
    }
};  // BaseOutputIterator



template<typename FinalT,
         typename SymT>
class BaseBidirIterator :   public BaseForwardIterator<FinalT, SymT>,
                            public mixins::PostfixDecOp<FinalT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef BaseForwardIterator<FinalT, SymT> Parent;
protected:
    /// Has to decrement iterator state.
    virtual void _V_decrement( Sym & ) = 0;
public:
    BaseBidirIterator() {}
    BaseBidirIterator( const Sym & s ) : BaseIterator<Sym>(s), Parent(s) {}
    virtual ~BaseBidirIterator() {}

    virtual Final & operator--() override {
        _V_decrement( Parent::sym() );
        return *this;
    }
};  // BaseForwardIterator


template<typename FinalT,
         typename SymT>
class DirectionComparableIterator : 
                            public BaseComparableIterator<FinalT, SymT>,
                            public mixins::FullComparableOp<FinalT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef DirectionComparableIterator<Final, Sym> Self;
    typedef BaseComparableIterator<Final, Sym> Parent;
protected:
    /// Has to return true if own state is greater than one provided as an
    /// argument.
    virtual bool _V_is_greater( const Self & ) const = 0;
public:
    DirectionComparableIterator() {}
    DirectionComparableIterator( const SymT & s ) : BaseIterator<Sym>(s), Parent(s) {}
    virtual ~DirectionComparableIterator() {}

    virtual bool operator>( const Self & prhs ) const override {
        return _V_is_greater( prhs.sym() ) > 0;
    }
};  // DirectionComparableIterator


// Direction:
//  - BaseForwardIterator<FinalT, SymT>
//  - BaseBidirIterator<FinalT, SymT>
// Access:
//  - BaseOutputIterator<FinalT, SymT, T>
//  - BaseInputIterator<FinalT, SymT, T>
// Comparison:
//  - BaseComparableIterator<FinalT, SymT>
//  - DirectionComparableIterator<FinalT, SymT>
template<
        template<class, class> class DirectionTT,
        template<class, class, class> class AccessTT,
        template<class, class> class ComparisonTT,
        typename FinalT,
        typename SymT,
        typename T,
        typename DistanceT=void>
class Iterator : public DirectionTT<FinalT, SymT>,
                 public AccessTT<FinalT, SymT, T>,
                 public ComparisonTT<FinalT, SymT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef T Value;
    typedef DistanceT Distance;

    typedef DirectionTT<Final, Sym> Direction;
    typedef AccessTT<Final, Sym, Value> Access;
    typedef ComparisonTT<Final, Sym> Comparison;
public:
    Iterator() {}
    Iterator( const Sym & s ) : BaseIterator<Sym>(s),
                                Direction(s),
                                Access(s),
                                Comparison(s) {}
    virtual ~Iterator() {}
};


template<template<class, class, class> class AccessTT,
         typename FinalT,
         typename SymT,
         typename T,
         typename DistanceT>
class Iterator<BaseBidirIterator,
               AccessTT,
               DirectionComparableIterator,
            FinalT,
            SymT,
            T,
            DistanceT> : public BaseBidirIterator<FinalT, SymT>,
                         public DirectionComparableIterator<FinalT, SymT>,
                         public mixins::PlusOp<FinalT, DistanceT>,
                         public mixins::DashOp<FinalT, DistanceT> {
public:
    typedef FinalT Final;
    typedef SymT Sym;
    typedef T Value;
    typedef DistanceT Distance;

    typedef BaseBidirIterator<Final, Sym> Direction;
    typedef AccessTT<Final, Sym, Value> Access;
    typedef DirectionComparableIterator<Final, Sym> Comparison;

    typedef Iterator<BaseBidirIterator, AccessTT, DirectionComparableIterator,
                     FinalT, SymT, T, DistanceT> Self;
protected:
    /// Has to compute and return distance between own state and state given as
    /// an argument.
    virtual Distance _V_distance( const Sym & ) const = 0;
    /// Has to increment own iterator state with given distance value.
    virtual void _V_advance( const Distance & ) = 0;

    /// Overrides interface method of DirectionComparableIterator with distance
    /// comparison.
    virtual bool _V_is_greater( const Self & prhs ) const override {
        return _V_distance( prhs.sym() ) > 0;
    }
public:
    Iterator() {}
    Iterator( const Sym & s ) : BaseIterator<Sym>(s),
                                Direction(s),
                                Access(s),
                                Comparison(s) {}
    virtual ~Iterator() {}

    virtual Final & operator+= ( const Distance & d ) override {
        _V_advance(  d );
        return *this;
    }

    virtual Final & operator-= ( const Distance & d ) override {
        _V_advance( -d );
        return *this;
    }

    virtual Distance operator- ( const Final & prhs ) const {
        return _V_distance( prhs );
    }
};

}  // namespace ::goo::iterators

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
    class ComparableIterator : public virtual IdentityOp<EndIteratorClassT>,
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

