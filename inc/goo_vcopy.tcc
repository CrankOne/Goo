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
template<typename BaseTypeT> BaseTypeT * clone( const BaseTypeT * );

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
         typename SelfTypeT,
         bool isAbstract=std::is_abstract<SelfTypeT>::value,
         bool isSame=std::is_same<BaseTypeT, SelfTypeT>::value>
class iDuplicable;


template<typename BaseTypeT,
         typename SelfTypeT >
class iDuplicable<BaseTypeT, SelfTypeT, true, true> {
protected:
    virtual BaseTypeT * _V_clone() const = 0;
    virtual const std::type_info & _V_cloning_type_id() const = 0;
};  // class iDuplicable


template<typename BaseTypeT,
         typename SelfTypeT>
class iDuplicable<BaseTypeT, SelfTypeT, true, false> : public BaseTypeT {
    //static_assert(std::is_copy_constructible<SelfTypeT>::value,
    //              "Outermost descendant type is not copy constructible.");
protected:
    virtual BaseTypeT * _V_clone() const override {
        return new SelfTypeT( *static_cast<const SelfTypeT *>(this) );
    }

    virtual const std::type_info & _V_cloning_type_id() const override {
        return typeid(*this);
    }

    friend BaseTypeT * ::goo::clone<>( const BaseTypeT * );
};  // class iDuplicable

template<typename BaseTypeT,
         typename SelfTypeT>
class iDuplicable<BaseTypeT, SelfTypeT, false, false> : public BaseTypeT {
    //static_assert(std::is_copy_constructible<SelfTypeT>::value,
    //              "Outermost descendant type is not copy constructible.");
protected:
    virtual BaseTypeT * _V_clone() const {
        return new SelfTypeT( *static_cast<const SelfTypeT *>(this) );
    }

    virtual const std::type_info & _V_cloning_type_id() const {
        return typeid(*this);
    }

    friend BaseTypeT * ::goo::clone<>( const BaseTypeT * );
};  // class iDuplicable

}  // namespace mixins

template<typename BaseTypeT>
BaseTypeT * clone( const BaseTypeT * origPtr ) {
    # ifndef NDEBUG
    if( origPtr->_V_cloning_type_id() != typeid(*origPtr) ) {
        emraise(dbgBadArchitect,
                "Instance %p of class \"%s\" manifests mismatching _V_cloning_type_id() "
                "(of class \"%s\") indicating Goo's coding style violation. Please, "
                "refer to \"#cloning\" section of Goo standard.");
    }
    # endif
    return origPtr->_V_clone();
}

}  // namespace goo

# endif  // H_GOO_VIRTUAL_COPY_CTR_H

