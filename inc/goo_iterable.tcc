# ifndef H_GOO_ITERABLE_MIXIN_H
# define H_GOO_ITERABLE_MIXIN_H

# include "goo_config.h"
# include "goo_exception.hpp"
# include "goo_mixins.tcc"
# include <iterator>

namespace goo {
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
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class IterableBase {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;
    typedef IterableBase<DereferencedType, IteratorAssociatedData, IteratorSym> Self;
protected:
    template<typename EndIteratorClassT>  /// Keep 'Iterator' name for default.
    class IteratorBase : public PostfixIncOp<EndIteratorClassT> {
    public:
        typedef Self Authority;
        typedef EndIteratorClassT EndIteratorClass;
    protected:
        Authority * _authorityPtr;  ///< no check for authority for performance.
        IteratorSym _sym;
        void _check_authority_is_set() const { DBG_NULLPTR_CHECK( _authorityPtr, "Empty iterator %p.", this ); }
        // Ctr/dtr --- protected from explicit use.
        IteratorBase() : _authorityPtr(nullptr) {}
    public:
        IteratorBase( const IteratorSym & d ) : 
                _authorityPtr(nullptr), _sym(d) {}

        // authority acquizition:
        Authority & authority() {
                _check_authority_is_set();
                return *_authorityPtr; }
        const Authority & authority() const {
                _check_authority_is_set();
                return *_authorityPtr; }

        // dereferencing:
        //IteratorAssociatedData & dereference() {
        //        _check_authority_is_set();
        //        return _authorityPtr->_V_dereference_sym( _sym );
        //    }
        //const IteratorAssociatedData & dereference() const {
        //        _check_authority_is_set();
        //        return _authorityPtr->_V_dereference_sym( _sym );
        //    }

        // increment:
        virtual EndIteratorClass & operator++() override {
            _check_authority_is_set();
            _authorityPtr->_V_increase_iterator_sym( _sym );
        }
    };
protected:
    virtual void _V_increase_iterator_sym( IteratorSym & ) = 0;
    //virtual IteratorAssociatedData _V_dereference_sym( const IteratorSym & ) = 0;
    //virtual const IteratorAssociatedData & _V_dereference_sym( const IteratorSym & ) const = 0;
};  // class IterableBase

//
// OUTPUT ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class OutputIterable : public virtual IterableBase<DereferencedTypeT,
                                                  IteratorAssociatedDataT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;

    typedef IterableBase<DereferencedType,
                         IteratorAssociatedData,
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
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class ComprIterable : public virtual IterableBase<DereferencedTypeT,
                                                  IteratorAssociatedDataT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;

    typedef IterableBase<DereferencedType,
                         IteratorAssociatedData,
                         IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class ComparableIterator : public virtual IdentityOp<EndIteratorClassT>,
                               public virtual Parent::template IteratorBase<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template IteratorBase<EndIteratorClassT> ParentIterator;

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
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class BidirIterable : public virtual ComprIterable<DereferencedTypeT,
                                                   IteratorAssociatedDataT,
                                                   IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;

    typedef ComprIterable<DereferencedType,
                          IteratorAssociatedData,
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
        virtual bool operator!= (const EndIteratorClass & eit) const override {
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_compare_iterators(
                        ParentIterator::_sym,
                        eit._sym );
        }
    };

    virtual void _V_decrease_iterator_sym( IteratorSym & ) = 0;
    virtual void _V_compare_iterators( const IteratorSym &, const IteratorSym & ) = 0;
};

//
// INPUT ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class InputIterable : public virtual ComprIterable<DereferencedTypeT,
                                                  IteratorAssociatedDataT,
                                                  IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;

    typedef ComprIterable<DereferencedType,
                          IteratorAssociatedData,
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
        DereferencedType && operator*() {  // TODO ??
            ParentIterator::_check_authority_is_set();
            return ParentIterator::_authorityPtr->_V_dereference_iterator_sym( ParentIterator::_sym );
        }
    };

    virtual const DereferencedType & _V_dereference_iterator_sym( const IteratorSym & ) const = 0;
    DereferencedType && _V_dereference_iterator_sym( const IteratorSym & ) = 0;  // TODO ??
};

//
// FORWARD ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorAssociatedDataT,
         typename IteratorSymT>
class ForwardIterable : public virtual ComprIterable<DereferencedTypeT,
                                                     IteratorAssociatedDataT,
                                                     IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;

    typedef ComprIterable<DereferencedType,
                          IteratorAssociatedData,
                          IteratorSym> Parent;
protected:
    template<typename EndIteratorClassT>
    class ForwardIterator : public virtual Parent::template ComparableIterator<EndIteratorClassT> {
    public:
        typedef EndIteratorClassT EndIteratorClass;
        typedef typename Parent::template ComparableIterator<EndIteratorClassT> ParentIterator;

        ForwardIterator(){}
    };

    // todo: begin/end/etc?
};

//
// RANDOM ACCESS ITERATOR MIXIN
//

template<typename DereferencedTypeT,
         typename IteratorAssociatedDataT,
         typename IteratorSymT,
         typename OffsetT>
class RandomIterable : public virtual BidirIterable<DereferencedTypeT,
                                                     IteratorAssociatedDataT,
                                                     IteratorSymT> {
public:
    typedef DereferencedTypeT DereferencedType;
    typedef IteratorAssociatedDataT IteratorAssociatedData;
    typedef IteratorSymT IteratorSym;
    typedef OffsetT Offset;

    typedef BidirIterable<DereferencedType,
                          IteratorAssociatedData,
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
}  // namespace goo

# endif  // H_GOO_ITERABLE_MIXIN_H

