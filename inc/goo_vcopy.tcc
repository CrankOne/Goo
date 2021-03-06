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

# ifndef H_GOO_VIRTUAL_COPY_CTR_H
# define H_GOO_VIRTUAL_COPY_CTR_H

# include <tuple>
# include <typeinfo>
# include "goo_utility.hpp"

namespace goo {

// fwd
namespace mixins {
/**@class iDuplicable
 * @brief Provides virtual copy ctr interface.
 *
 * This template class provides interfacing functions for copy construction
 * (cloning) without knoledge of concrete class. Base class is specified as
 * a template parameter.
 *
 * By default, classes which does not implement a copy constructor will cause
 * a SFINAE (compilation failure) indicating that user code have to provide
 * it.
 *
 * @ingroup utils
 */
template<typename BaseTypeT,
         typename SelfTypeT=BaseTypeT,
         typename ParentT=BaseTypeT,
         bool forceImplement=false,
         bool isSame=std::is_same<BaseTypeT, SelfTypeT>::value>
class iDuplicable;
}  // namespace mixins

template<typename BaseTypeT, typename SelfTypeT, typename ParentT, bool forceImplem> BaseTypeT *
clone( const mixins::iDuplicable<BaseTypeT, SelfTypeT, ParentT, forceImplem> * origPtr );

namespace mixins {

//
// An iDuplicable implementation for abstract base class
// (copy ctr is not provided). Designed to be used in base
// classes.
template<typename BaseTypeT,
         typename SelfTypeT,
         typename ParentT>
class iDuplicable<BaseTypeT, SelfTypeT, ParentT, false, true> {
public:
    typedef iDuplicable<BaseTypeT, SelfTypeT, ParentT, false, true> DuplicableParent;
    typedef iDuplicable<BaseTypeT, SelfTypeT> DuplicableBase;
    typedef ParentT Parent;

    virtual ~iDuplicable(){}
protected:
    /// Pure virtual method for copy construction.
    virtual BaseTypeT * _V_clone() const = 0;

    /// Returns standard C++ RTTI struct descibing this instance class
    /// (outermost descendant).
    virtual const std::type_info & _V_cloning_type_id() const {
        return typeid( SelfTypeT );
    }

    friend BaseTypeT * ::goo::clone<>( const DuplicableParent * origPtr );
};  // class iDuplicable

//
// An iDuplicable implementation for base class with forced cloning
// method implementation (copy ctr is provided). Designed to be used
// in base classes which have to be instantiated.
template<typename BaseTypeT,
         typename SelfTypeT,
         typename ParentT>
class iDuplicable<BaseTypeT, SelfTypeT, ParentT, true, true> {
public:
    typedef iDuplicable<BaseTypeT, SelfTypeT, ParentT, true, true> DuplicableParent;
    typedef iDuplicable<BaseTypeT, SelfTypeT> DuplicableBase;
    typedef ParentT Parent;

    virtual ~iDuplicable(){}
protected:
    /// Performs invokation of copy constructor.
    virtual BaseTypeT * _V_clone() const {
        return new SelfTypeT( *static_cast<const SelfTypeT *>(this) );
    }

    /// Returns standard C++ RTTI struct descibing this instance class
    /// (outermost descendant).
    virtual const std::type_info & _V_cloning_type_id() const {
        return typeid( SelfTypeT );
    }

    friend BaseTypeT * ::goo::clone<>( const DuplicableParent * origPtr );
};  // class iDuplicable

//
// An iDuplicable implementation for outern descendant with
// abstract base class (copy ctr is provided).
template<typename BaseTypeT,
         typename SelfTypeT,
         typename ParentT>
class iDuplicable<BaseTypeT, SelfTypeT, ParentT, false, false> : public ParentT {
public:
    typedef iDuplicable<BaseTypeT, SelfTypeT, ParentT, false, false> DuplicableParent;
    typedef iDuplicable<BaseTypeT, BaseTypeT> DuplicableBase;
    typedef ParentT Parent;

    virtual ~iDuplicable(){}
protected:
    template<typename ... CtrArgsT>
    iDuplicable( CtrArgsT ... args ) : ParentT( args ... ) {}

    template<typename T=typename std::is_default_constructible<ParentT>::type>
    iDuplicable() : ParentT() {}

    /// Returns standard C++ RTTI struct descibing this instance class
    /// (outermost descendant).
    virtual const std::type_info & _V_cloning_type_id() const override {
        return typeid( SelfTypeT );
    }

    /// Forces downcast to avoid stack overflow araised due to
    /// default behaviour (bug?).
    iDuplicable( const SelfTypeT & self ) : Parent( self ) {}

    // Performs invokation of copy constructor.
    virtual BaseTypeT * _V_clone() const override {
        # if 0
        static_assert( std::is_base_of<DuplicableBase, Parent>::value,
                        "Outermost descendant has no abstract duplicable base." );
        // Copy ctr have to be defined as cloning procedure uses it.
        static_assert(std::is_copy_constructible<SelfTypeT>::value,
                        "Outermost descendant type has no copy construcor (see SelfTypeT).");
        // Outermost class have to be instantiable as we're going to create
        // copies.
        static_assert(!std::is_abstract<SelfTypeT>::value,
                        "Outermost type is abstract.");
        # endif
        return new SelfTypeT( *static_cast<const SelfTypeT *>(this) );
    }
    friend BaseTypeT * ::goo::clone<>( const DuplicableParent * origPtr );
};  // class iDuplicable

}  // namespace mixins

template<typename BaseTypeT, typename SelfT, typename ParentT, bool ForceImplem> BaseTypeT *
clone( const mixins::iDuplicable<BaseTypeT, SelfT, ParentT, ForceImplem> * origPtr ) {
    # ifndef NDEBUG
    if( origPtr->_V_cloning_type_id() != typeid(*origPtr) ) {
        emraise(dbgBadArchitect,
                "Instance %p of class \"%s\" manifests mismatching _V_cloning_type_id() "
                "(of class \"%s\") indicating Goo's coding style violation. Please, "
                "refer to \"#cloning\" section of Goo standard.",
                origPtr, typeid(*origPtr).name(),
                origPtr->_V_cloning_type_id().name() );
    }
    # endif
    return origPtr->_V_clone();
}

template<typename BaseTypeT,
         typename DesiredT,
         typename ProvidedT>
DesiredT * clone_as( const ProvidedT * origProvidedPtr ) {
    return static_cast<DesiredT *>(clone<BaseTypeT>(
           static_cast<const mixins::iDuplicable<BaseTypeT> *>(origProvidedPtr) 
                                                   )
                                  );
}

}  // namespace goo

# endif  // H_GOO_VIRTUAL_COPY_CTR_H

