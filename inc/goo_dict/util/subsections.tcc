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

/*
 * Created by crank on 06.02.18.
 */

#ifndef H_GOO_DICT_SUBSECTIONS_H
#define H_GOO_DICT_SUBSECTIONS_H

# include "goo_dict/types.hpp"
# include "goo_dict/generic_dict.tcc"

namespace goo {
namespace dict {
namespace aux {

template< typename KeyT
        , template<class, class> class THashT
        , typename SubsectionT>
struct iSubsections : protected THashT<KeyT, SubsectionT*> {
    typedef KeyT Key;
    typedef SubsectionT Subsection;
    typedef THashT<Key, Subsection *> Parent;
    typedef iSubsections<KeyT, THashT, SubsectionT> Self;

    const Parent & subsections() const {
        return *this;
    }

    // First level getters --- provide basic access to subsections. Access
    // to parameters is provided by the GenericDictionary<>::entry_ptr().

    /// Tries to retrieve a pointer to the subsection instance by name. If
    /// no subsection with such a name found, returns null pointer. Does not
    /// perform path expansion.
    virtual const Subsection * subsection_ptr( const KeyT & name ) const {
        auto it = Parent::find( name );
        if( Parent::end() == it ) {
            return nullptr;
        }
        return it->second;
    }

    /// Non-const version of same-named method.
    virtual Subsection * subsection_ptr( const KeyT & name ) {
        const auto * cThis = this;
        return const_cast<Subsection *>(cThis->subsection_ptr( name ));
    }

    // Second level getters --- provide access by references.

    /// Returns reference to subsection with given name or raises
    /// goo::notFound.
    virtual const Subsection & subsection( const KeyT & name ) const {
        auto ptr = static_cast<const Subsection*>(this)->subsection_ptr(name);
        if( !ptr ) {
            emraise( notFound, "No subsection \"%s\" found in"
                    " dictionary %p.", KeyTraits<KeyT>::to_str(name).c_str(), this );
        }
        return *ptr;
    }

    /// Returns reference to subsection with given name or raises
    /// goo::notFound.
    virtual Subsection & subsection( const KeyT & name ) {
        const auto * cThis = this;
        return const_cast<Subsection &>(cThis->subsection(name));
    }

    // Traversing methods

    /// Template function performing iterative invocation of callable for each
    /// subsection entry in current dictionary (mutable). The callable signature
    /// must accept the iterator argument.
    template<typename CallableT>
    CallableT each_subsection_revise( CallableT c ) {
        for( auto it = THashT<KeyT, SubsectionT*>::begin()
           ; THashT<KeyT, SubsectionT*>::end() != it ; ++it ) {
            c( it );
        }
        return c;
    }

    /// Template function performing iterative invocation of callable for each
    /// subsection entry in current dictionary (mutable). The callable signature
    /// must accept the iterator argument.
    template<typename CallableT>
    CallableT each_subsection_read( CallableT c ) const {
        for( auto it = THashT<KeyT, SubsectionT*>::begin()
           ; THashT<KeyT, SubsectionT*>::end() != it ; ++it ) {
            c( it );
        }
    }

    template<typename CallableT>
    struct RecursiveReadingVisitor {
        typedef typename THashT<KeyT, SubsectionT*>::const_iterator SubsectionIterator;

        CallableT _c;

        virtual void operator()( SubsectionIterator it ) {
            it->second->each_entry_read( _c );
            it->second->each_subsection_read( *this );
        }

        RecursiveReadingVisitor( CallableT c ) : _c(c) {}
    };

    template<typename CallableT>
    struct RecursiveRevisingVisitor {
        typedef typename THashT<KeyT, SubsectionT*>::iterator SubsectionIterator;

        CallableT _c;

        virtual void operator()( SubsectionIterator it ) {
            it->second->each_entry_revise( _c );
            it->second->each_subsection_revise( *this );
        }

        RecursiveRevisingVisitor( CallableT c ) : _c(c) {}
    };
};

}  // namespace aux
}  // namespace dict
}  // namespace goo

#endif  // H_GOO_DICT_SUBSECTIONS_H
