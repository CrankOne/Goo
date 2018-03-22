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
# include "goo_sys/exception.hpp"

namespace goo {

/**@brief Type traits struct defining how to copy derived classes for certain
 * base.
 *
 * Override this traits with specialization for various operations that needed
 * to be done upon copy contruction (instance-based allocators, etc.).
 */
template< typename BaseT
        , typename ... CCtrArgsT>
struct CloningTraits {
    template<typename DerivedT>
    struct ForDerived {
        static BaseT * clone( const DerivedT & o, CCtrArgsT ... ccargs ) {
            return new DerivedT( o, ccargs... );
        }
    };
};

// fwd
namespace mixins {
/**@class iDuplicable
 * @brief Provides virtual copy ctr interface.
 *
 * This template class provides interfacing functions for copy construction
 * (cloning) without knowledge of concrete class. Base class is specified as
 * a template parameter.
 *
 * By default, classes which does not implement a copy constructor will cause
 * a SFINAE (compilation failure) indicating that user code have to provide
 * it.
 *
 * User code may rely on automatic cloning method implementation when target
 * class provides a copy constructor. In that case, the fourth argument has to
 * be set to `false`.
 *
 * For goo devs: please note that despite in most of the cases this fourth
 * argument could be set to
 * `std::is_copy_constructible<T>::value && !std::is_abstract::value` we can not
 * provide this expression as a default argument since SelfTypeT is not known
 * upon this template instantiation.
 *
 * @ingroup utils
 */
template< typename BaseTypeT
        , typename SelfTypeT=BaseTypeT
        , typename ParentT=BaseTypeT
        , typename ... CopyCtrArgTs
        >
class iDuplicable;
}  // namespace mixins

/**@brief Function notation for virtual copy ctr.
 *
 * An alias for the virtual copy constructor. Merely a
 * procedural shim for iDuplicable<BaseTypeT, SelfTypeT, ParentT>::clone()
 * method.
 */
template< typename BaseTypeT, typename ... CopyCtrArgTs> BaseTypeT *
clone( const BaseTypeT * origPtr, CopyCtrArgTs ... );

namespace mixins {

/// An iDuplicable implementation for abstract base class
/// (copy ctr is not provided). Designed to be used in base
/// classes. The forceImplement is always false at this level (there is nothing
/// to 'force' actually, nothing to be automatically implemented).
template< typename SelfTypeT, typename ... CopyCtrArgTs>
class iDuplicable<SelfTypeT, SelfTypeT, SelfTypeT, CopyCtrArgTs...> {
public:
    //typedef iDuplicable<SelfTypeT, SelfTypeT, SelfTypeT, CopyCtrArgTs...> DuplicableParent;
    typedef iDuplicable<SelfTypeT, SelfTypeT, SelfTypeT, CopyCtrArgTs...> DuplicableBase;
    typedef SelfTypeT Parent;
    typedef SelfTypeT BaseType;

    virtual ~iDuplicable(){}
protected:
    /// Pure virtual method for copy construction.
    virtual BaseType * _V_clone( CopyCtrArgTs ... ) const = 0;

    /// Returns standard C++ RTTI struct descibing this instance class
    /// (outermost descendant).
    virtual const std::type_info & _V_cloning_type_id() const {
        return typeid( SelfTypeT );
    }

    friend BaseType * ::goo::clone<>( const BaseType *, CopyCtrArgTs ... );
};  // class iDuplicable

/// An iDuplicable implementation for outermost descendant with
/// abstract base class (copy ctr is provided).
template< typename BaseTypeT
        , typename SelfTypeT
        , typename ParentT
        , typename ... CopyCtrArgTs>
class iDuplicable : public ParentT {
public:
    /// Parent class typedef for subclass usage.
    typedef ParentT Parent;
    /// Cloning self typedef for subclass usage.
    typedef iDuplicable<BaseTypeT, SelfTypeT, ParentT, CopyCtrArgTs ...> DuplicableSelf;
    /// Cloning abstract base typedef for subclass usage.
    typedef BaseTypeT BaseType;
    /// Most-base class in inheritance hierarchy typedef for subclass usage.
    typedef iDuplicable<BaseTypeT, BaseTypeT, BaseTypeT, CopyCtrArgTs ...> DuplicableBase;
    /// Trivial virtual dtr.
    virtual ~iDuplicable(){}
protected:
    /// Forwarding ctr. Does nothing except directo forwarding  of provided args
    /// to parent class.
    template<typename ... CtrArgsT>
    iDuplicable( CtrArgsT ... args ) : ParentT( args ... ) {}

    /// Returns standard C++ RTTI struct describing this instance class
    /// (outern descendant).
    virtual const std::type_info & _V_cloning_type_id() const override {
        return typeid( SelfTypeT );
    }

    /// Forces upcast to avoid stack overflow appearing due to
    /// default behaviour.
    iDuplicable( const SelfTypeT & self, CopyCtrArgTs ... ccargs )
                : ParentT( static_cast<const ParentT &>(self), ccargs ... ) {}

    /// Invokes copy constructor.
    virtual BaseTypeT * _V_clone( CopyCtrArgTs ... ccargs ) const override {
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
        //return new SelfTypeT( *static_cast<const SelfTypeT *>(this), ccargs ... );
        return CloningTraits<BaseTypeT, CopyCtrArgTs...>::template ForDerived<SelfTypeT> \
                    ::clone( static_cast<const SelfTypeT &>(*this), ccargs... );
    }

    friend BaseType * ::goo::clone<>( const BaseType *, CopyCtrArgTs ... );
};  // class iDuplicable

}  // namespace mixins

template< typename BaseTypeT
        , typename ... CopyCtrArgTs>
# if 0
typename std::enable_if< std::is_base_of< mixins::iDuplicable< BaseTypeT
                                                             , BaseTypeT
                                                             , BaseTypeT
                                                             , CopyCtrArgTs... >
                                        , BaseTypeT
                                        >::value
                       , BaseTypeT * >::type
# else
BaseTypeT *
# endif
clone( const BaseTypeT * orig, CopyCtrArgTs ... ccargs) {
    # ifndef NDEBUG
    if( orig->_V_cloning_type_id() != typeid(*orig) ) {
        emraise( dbgBadArchitect,
                "Instance %p of class \"%s\" manifests mismatching _V_cloning_type_id() "
                "(of class \"%s\") indicating Goo's coding style violation. Please, "
                "refer to \"#cloning\" section of Goo standard.",
                orig, typeid(*orig).name(),
                orig->_V_cloning_type_id().name() );
    }
    # endif
    return orig->_V_clone( ccargs ... );
}

template< typename BaseTypeT
        , typename DesiredT
        , typename ProvidedT
        , typename ... CopyCtrArgTs>
DesiredT * clone_as( const ProvidedT * origProvidedPtr
                   , CopyCtrArgTs ... ccargs) {
    return static_cast<DesiredT *>(clone<BaseTypeT>(
           static_cast< const BaseTypeT * >(origProvidedPtr)
         , ccargs ...
                                                   )
                                  );
    /*const mixins::iDuplicable< BaseTypeT
                                                , BaseTypeT
                                                , BaseTypeT
                                                , CopyCtrArgTs ... >*/
}

}  // namespace goo

# endif  // H_GOO_VIRTUAL_COPY_CTR_H

