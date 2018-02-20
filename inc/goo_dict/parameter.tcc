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

# ifndef H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

# include "goo_types.h"
# include "goo_exception.hpp"
# include "value.hpp"

# include <list>
# include <vector>

# include <cassert>
# include <sstream>

namespace goo {
namespace dict {

# if 1

//class iSingularParameter;

//template<typename ValueT>
//class iParameter;

/**@class Parameter
 * @brief User-side implementation class. Extension point for user parameters type
 *
 * There is no default implementation of this class --- only standard C classes
 * are implemented in Goo library.
 */
template< typename ValueT
        , typename ... AspectTs>
class Parameter : public mixins::iDuplicable< iAbstractValue
                                           , Parameter<ValueT, AspectTs...>
                                           , TValue<ValueT, AspectTs...> > {
public:
    typedef mixins::iDuplicable< iAbstractValue
                               , Parameter<ValueT, AspectTs...>
                               , TValue<ValueT, AspectTs...> > DuplicableParent;
    template<typename ... Ts> Parameter(Ts ... ctrArgs) : DuplicableParent(ctrArgs...) {}
};

template <typename T, typename ... AspectTs> class IntegralParameter;
template <typename T, typename ... AspectTs> class FloatingPointParameter;
template <typename T, typename ... AspectTs> class EnumParameter;
template <typename T, typename ... AspectTs> class PointerParameter;

template< typename T
        , typename ... AspectTs >
using InsertableParameter = typename
    std::conditional< std::is_arithmetic<T>::value,
        typename std::conditional< std::is_same<T, bool>::value,
            Parameter<bool, AspectTs...>,
            typename std::conditional< std::is_integral<T>::value,
                IntegralParameter<T, AspectTs...>,
                typename std::conditional< std::is_floating_point<T>::value,
                    FloatingPointParameter<T, AspectTs...>,
                    Parameter<T, AspectTs...>
                    >::type
            >::type
        >::type,
        typename std::conditional< std::is_enum<T>::value,
            EnumParameter<T, AspectTs...>,
            typename std::conditional< std::is_pointer<T>::value,
                PointerParameter<T, AspectTs...>,
                Parameter<T, AspectTs...>
                >::type
            >::type
    >::type;

/**@brief A parameter list class.
 * 
 * This template class implements a parameter list for any defined singular
 * parameter type. The types have to be homogeneous in list (i.e. one can not
 * specify a list consisting of, e.g. boolean and floating point number types).
 *
 * Prameter list may have default values defined as an C++11 initializer list
 * upon construction. Default values will be deleted if at least one value
 * will override the parameter.
 *
 * Note about `_setToDefault` property: once it was set to true, the next
 * appending operation will cause `_values` list to be cleared. This flag may
 * be useful not only with the initial configuration, but also for overriding
 * or appending variables set in config files.
 */
template< typename ValueT
        , typename ... AspectTs>
class Parameter<Array<ValueT>, AspectTs... > :
            public mixins::iDuplicable< iAbstractValue
                                     ,  Parameter< Array<ValueT>, AspectTs ... >
                                     ,  /*protected?*/ InsertableParameter< ValueT, AspectTs ... > > {
public:
    typedef mixins::iDuplicable< iAbstractValue
                               , Parameter< Array<ValueT>, AspectTs ... >
                               , InsertableParameter<ValueT, AspectTs ...> > DuplicableParent;
private:
    bool _setToDefault;
    Array<ValueT> _values;
protected:
    virtual void _V_push_value( const ValueT & v ) {
        if( _setToDefault ) {
            _values.clear();
            _setToDefault = false;
        }
        _values.push_back( v ); }

    //virtual void _V_parse_argument( const char * strval ) override {
    //    InsertableParameter<ValueT>::_V_parse_argument( strval );
    //    _V_push_value( InsertableParameter<ValueT>::value() ); }
public:
    const Array<ValueT> & values() const { return _values; }

    template<class ... Types>
    Parameter( Types ... args ) :
        DuplicableParent( args ... ),
        _setToDefault( false ) {}

    ~Parameter() {}

    /// Returns `_setToDefault` flag.
    bool is_set_to_default() const { return _setToDefault; }

    /// Sets `_setToDefault` flag. See note about this flag in class
    /// description.
    void set_to_default(bool v) { _setToDefault = v; }

    /// This method is usually used by some user code desiring set all the
    /// parameters list at once. It should not be used by Goo API, during the
    /// normal argument parsing cycle.
    void assign( const Array<ValueT> & plst ) {
        for( auto v : plst ) {
            _V_push_value(v);
        }
    }

    friend class DictInsertionProxy;

    //using AbstractParameter::name;
    //using AbstractParameter::description;
    //using AbstractParameter::shortcut;
    // ... whatever?
};

/// This specification has to be treated in the special manner, in a similar way
/// to dicitonary.
/// Include "goo_dict/los.hpp" header for full declaration.
template<typename ... AspectTs> class Parameter<List<iBaseValue<AspectTs...>*> >;

template<typename ... AspectTs>
template<typename T> const Array<T> &
iBaseValue<AspectTs...>::as_array_of() const {
    # if 1
    typedef InsertableParameter< Array<T> > const * CastTarget;
    auto ptr = dynamic_cast<CastTarget>(this);
    if( !ptr ) {
        goo_badcast( typename DECLTYPE(this)
                   , CastTarget
                   , this
                   , "Type mismatch for parameters dictionary array entry." );
    }
    return ptr->values();
    # endif
}

template<typename ... AspectTs>
template<typename T>
typename std::enable_if< ! stdE::is_one_of<T, AspectTs...>::value, const T &>::type
iBaseValue<AspectTs...>::as() const {
    typedef TValue<T, AspectTs...> const * CastTarget;
    auto ptr = dynamic_cast<CastTarget>(this);
    if( !ptr ) {
        goo_badcast( typename DECLTYPE(this)
                   , CastTarget
                   , this
                   , "Type mismatch for parameters dictionary scalar entry." );
        # if 0
        const iSingularParameter * namedP = dynamic_cast<const iSingularParameter *>(this);
        if( namedP ) {
            if( namedP->name() ) {
                goo_badcast( DECLTYPE(this), CastTarget, this, "Parameter name: \"%s\"."
                           , namedP->name() );
            } else if( namedP->has_shortcut() ) {
                goo_badcast( DECLTYPE(this), CastTarget, this, "Parameter shortcut: \"%c\"."
                           , namedP->shortcut() );
            }
        }
        goo_badcast( DECLTYPE(this), CastTarget, this, "Anonymous parameter." );
        # endif
    }
    return ptr->value();
}

# endif

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

