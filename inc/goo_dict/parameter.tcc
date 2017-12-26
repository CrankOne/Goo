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
# include "goo_vcopy.tcc"

# include <list>
# include <vector>

# include <cassert>
# include <sstream>

namespace goo {
namespace dict {

/// Sequence container template. The particular selection may significantly
/// affect performance or memory construction. Since goo::dict is generally
/// designed for application configuration that occurs once, the performance
/// matters less than memory consumption that is slightly more efficient for
/// list sequencies (TODO: has to be checked on benchmarks, actually).
template<typename T> using List = std::vector<T>;

class DictInsertionProxy;
class iSingularParameter;

template<typename ValueT>
class iParameter;

// Type-agnostic parameter interface classes
// ////////////////////////////////////////

/**@class iAbstractParameter
 * @brief An abstract parameter is a base class for Goo's
 * dictionary entires.
 *
 * We're tending to follow here to POSIX convention:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 * extended with GNU long options:
 * https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html ,
 *
 * so the option-arguments are never optional
 * (see Guideline 7 of 12.2). That means one can not, for example, declare
 * an option `-c` with optional argument. Even if the `-c` option is a
 * logical flag the option-argument should either be necessarily presented,
 * or prohibited by declaration. In first case this option have to be
 * provided either as `-c true` or `-c false` (or in any equivalent
 * appropriate way for options of logic type). In the second case this option
 * can not receive an option-argument (and should be provided as `-c` or
 * omitted).
 *
 * The following subclasses are expected:
 *  - single-char keys (here referred as `shortcuts')   (flags or with values, multiple time)
 *  - long options                                      (flags or with values)
 *  - unnamed positional arguments
 *  - sections (dictionaries itself)
 *
 * Flag can not be required (required=false)
 *
 * Required can not be set (set=false)
 *
 * @ingroup appParameters
 */
class iAbstractParameter : public mixins::iDuplicable<iAbstractParameter> {
public:
    typedef UByte ParameterEntryFlag;
    static const ParameterEntryFlag
            set,            ///< Value is set (=false on init for required/flag/dict).
            flag,           ///< Two-state logical parameter without an argument.
            positional,     ///< Positional argument (=false for flag/dict).
            atomic,         ///< Indicates true parameter (otherwise, it's a dictionary).
            singular,       ///< Can be provided only once (=false for dict).
            required,       ///< A mandatory parameter entry (=false for flag/dict).
            shortened       ///< Has a shortcut (single-letter option, =false for positional).
        ;
private:
    char * _name,           ///< Name of the option. Can be set to nullptr.
         * _description;    ///< Description of the option. Can be set to nullptr.
    /// Stores logical description of an instance.
    ParameterEntryFlag _flags;
protected:
    /// Used only when shortened flag is set.
    char _shortcut;
protected:
    /// Sets the "set" flag translating instance to initialized state.
    void _set_is_set_flag();

    /// Sets the "is flag" flag.
    void _set_is_flag_flag();

    /// This method is to be used by lists only.
    void _unset_singular();

    /// Single ctr can be only invoked by descendants.
    iAbstractParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0');

    /// Copy ctr.
    iAbstractParameter( const iAbstractParameter & );

    /// Raises an exception if contradictory states are set for parameter
    /// on initial stage.
    void _check_initial_validity();

    /// Useful for auxilliary classes.
    void _append_description( const char * );
public:
    virtual ~iAbstractParameter();

    /// Returns pointer to name string.
    const char * name() const;

    /// Name setter. Use with care since these parameters may often be cached
    /// by owning dictionaries.
    void name( const char * );

    /// Returns pointer to description string.
    const char * description() const;

    /// Returns shortcut if it was set. Otherwise returns '\0'.
    char shortcut() const { return _shortcut; }

    /// Returns true, if parameter has a value set (even if it is a default one).
    bool is_set() const {
            return _flags & set;
        }
    /// Returns true, if parameter means logical flag.
    bool is_flag() const {
            return _flags & flag;
        }
    /// Returns true, if parameter expects a value (or already contain a default one).
    bool requires_value() const {
            return !is_flag();
        }
    /// Returns true, if parameter has no name (at all --- even one-letter shortcut).
    bool is_positional() const {
            return _flags & positional;
        }
    /// Returns true, if parameter is not a dictionary.
    bool is_atomic() const {
            return _flags & atomic;
        }
    /// Returns true, if parameter is a dictionary.
    bool is_dictionary() const {
            return !is_atomic();
        }
    /// Returns true, if parameter has only one value.
    bool is_singular() const {
            return _flags & singular;
        }
    /// Returns true, if parameter has a set of values.
    bool has_multiple_values() const {
            return !is_singular();
        }
    /// Returns true, if parameter is mandatory.
    bool is_mandatory() const {
            return _flags & required;
        }
    /// Returns true, if parameter can be omitted.
    bool is_optional() const {
            return !is_mandatory();
        }
    /// Returns true, if parameter has a single-character shortcut.
    bool has_shortcut() const {
            return _flags & shortened;
        }

    /// Sets the "required" flag marking a mandatory parameter.
    void set_is_argument_required_flag();

    friend class ::goo::dict::DictInsertionProxy;
}; // class iAbstractParameter

/**@brief Type-agnostic value parameter base.
 *
 * This abstract base claims interface of type-agnostic value-bearing parameter
 * instance providing generic methods accessing the data memory address and
 * C++ RTTI information.
 * */
class iBaseValue {
protected:
    /// Shall return untyped data pointer.
    virtual void * _V_data_addr() = 0;
    /// Shall return untyped (const) data pointer.
    virtual const void * _V_data_addr() const = 0;
    /// Shall return C++ RTTI type info.
    virtual const std::type_info & _V_target_type_info() const = 0;
public:
    /// Returns untyped data pointer.
    void * data_addr() { return _V_data_addr(); }
    /// Returns untyped (const) data pointer.
    const void * data_addr() const { return _V_data_addr(); }
    /// Returns C++ RTTI value type info.
    virtual const std::type_info & target_type_info() const
                { return _V_target_type_info(); }

    // note: for this two methods see implementation at the end of the header.
    // They use downcast operations with types that are incomplete at the
    // moment.
    /// Getter method (convinience).
    template<typename T> const T & as() const;
    /// Getter method (convinience) for list parameters.
    template<typename T> const List<T> & as_array_of() const;
    // TODO: ^^^ rename to as_array_of
};  // class iBaseValue


/**@class iStringConvertibleParameter
 * @brief Generic interface for parameter instance that may be converted to str.
 *
 * This is a data type-agnostic part of the string-convertible parameter class.
 */
class iStringConvertibleParameter : virtual public iBaseValue {
protected:
    /// Shall translate string expression to C++ value.
    virtual void _V_parse_argument( const char * ) = 0;
    /// Shall render C++ value to string.
    virtual std::string _V_to_string() const = 0;
public:
    /// Translates value from string representation.
    void parse_argument( const char * strval ) {  _V_parse_argument( strval ); }
    /// Renders string from C++ value.
    std::string to_string() const { return _V_to_string(); }
    template<typename ValueT, typename EnableT=void> struct ConversionTraits;
    //{
    //    typedef ValueT Value;
    //    static Value parse_string_expression( const char * stv ) {
    //        Value v;
    //        std::ostringstream ss(stv);
    //        ss >> v;
    //        return v;
    //    }
    //    static std::string to_string_expression( const Value & v ) {
    //        std::stringstream ss;
    //        ss << v;
    //        return ss.str();
    //    }
    //};
};  // class iStringConvertibleParameter

/**@class iSingularParameter
 * @brief Generic parameter features interface.
 *
 * At this level of inheritance the less-generic definition of
 * parameter dictionary entries is defined.
 *
 * @ingroup appParameters
 */
class iSingularParameter : public iAbstractParameter
                         , virtual public iStringConvertibleParameter {
protected:
    virtual void _V_assign( const iSingularParameter & ) = 0;
public:
    iSingularParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0' );

    virtual void assign( const iSingularParameter & src ) {
        _V_assign( src );
    }

    iSingularParameter & operator=( const iSingularParameter & src ) {
        assign( src );
        return *this;
    }
};  // class iSingularParameter


//
// Known type-handling abstraction layer
// ////////////////////////////////////


/**@brief Template container for a value.
 * @class iTValue
 *
 * Represents a value-keeping aspect of the parameter classes.
 * */
template<typename ValueT>
class iTValue : public virtual iBaseValue {
public:
    typedef ValueT Value;
private:
    Value _value;
protected:
    /// Mutable value reference getter.
    ValueT & _mutable_value() { return _value; }
    virtual const std::type_info & _V_target_type_info() const final {
        return typeid(ValueT); }
    /// This method is to be used by user code, for special cases. It should
    /// not be used by Goo API, during the normal argument parsing cycle.
    virtual void _set_value( const ValueT & v ) { _value = v; }
    /// Returns kept value address.
    virtual void * _V_data_addr() override { return &_value; }
    /// Returns kept value address (const).
    virtual const void * _V_data_addr() const override { return &_value; }
    /// Potentially dangerous ctr leaving the value uninitialized.
    iTValue() : _value() {}
public:
    /// Const value getter (public).
    virtual const ValueT & value() const { return _value; }
    explicit iTValue( const ValueT & v ) : _value(v) {}
    iTValue( const iTValue<Value> & o ) : _value(o._value) {}
};

/**@brief Default template implementating value parameters that may be converted
 * to string.
 *
 * One may be interested in partial specialization of this class to support
 * various formatting modifiers.
 * */
template<typename ValueT>
class iTStringConvertibleParameter : virtual public iStringConvertibleParameter
                                   , public iTValue<ValueT> {
public:
    typedef ValueT Value;
    typedef typename iStringConvertibleParameter::ConversionTraits<Value> ValueTraits;
    typedef iTStringConvertibleParameter<ValueT> Self;
protected:
    /// Sets the value kept by current instance from string expression.
    virtual void _V_parse_argument( const char * strval ) override {
        this->_set_value( ValueTraits::parse_string_expression( strval ) );
    }
    /// Expresses the value kept by current instance as a string.
    virtual std::string _V_to_string( ) const override {
        return ValueTraits::to_string_expression( this->value() );
    }
    /// Potentially dangerous ctr leaving the value uninitialized.
    iTStringConvertibleParameter() : iTValue<Value>() {}
public:
    iTStringConvertibleParameter( const ValueT & v ) : iTValue<Value>(v) {}
    iTStringConvertibleParameter( const Self & o ) : iTValue<Value>( o ) {}
};


/**@class iParameter
 * @brief A valued intermediate class representing dictionary entry with
 *        value of certain type.
 *
 * Junction class of the iTStringConvertibleParameter template with
 * iSingularParameter defining annotated parameter structure used by
 * parameter dictionaries (annotated hashing containers).
 * */
template<typename ValueT>
class iParameter : public iSingularParameter
                 , public iTStringConvertibleParameter<ValueT> {
public:
    typedef iTStringConvertibleParameter<ValueT> ValueStoringType;
    typedef typename ValueStoringType::Value Value;
    typedef iSingularParameter::ParameterEntryFlag ParameterEntryFlag;
protected:
    virtual void _set_value( const ValueT & ) override;
    virtual void _V_assign( const iSingularParameter & ) override;
public:
    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ = '\0' );

    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ /*= '\0' allowed */,
                const ValueT & defaultValue );

    iParameter( const iParameter<ValueT> & );

    ~iParameter() {}

    /// Const value getter (public). Raises "unitialized" error if value was
    /// not set.
    virtual const ValueT & value() const;
};  // class iParameter

template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description,
                                ParameterEntryFlag flags,
                                char shortcut_ ) :
        iSingularParameter( name, description, flags, shortcut_ ) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description,
                                ParameterEntryFlag flags,
                                char shortcut_,
                                const ValueT & defaultValue ) :
        iSingularParameter( name, description, flags, shortcut_ ),
        ValueStoringType(defaultValue) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const iParameter<ValueT> & o ) :
        iSingularParameter(o),
        ValueStoringType(o) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT> const ValueT &
iParameter<ValueT>::value() const {
    if( !is_set() ) {
        char vlOnShortcut[2] = {'\0', '\0'};
        if( !name() && has_shortcut() ) {
            vlOnShortcut[0] = shortcut();
        }
        throw goo::TheException<goo::Exception::uninitialized>(
                name() ? name() : vlOnShortcut,
                &(value()), "Parameter value is not set." );
    }
    return ValueStoringType::value();
}

template<typename ValueT> void
iParameter<ValueT>::_V_assign( const iSingularParameter & spVal ) {
    if( !spVal.is_set() ) {
        // The original value is not set. The expected behaviour in this case
        // is to do nothing with own value. TODO: this has to be reflected in
        // documentation!
        return;
    }
    iSingularParameter::_set_is_set_flag();

    // TODO: dynamic_cast<> here may ruin the performance for complex types.
    auto p = dynamic_cast<const iParameter<ValueT> *>( &spVal );
    if( !p ) {
        emraise( badCast, "Types mismatch. Unable to perform assignment of "
            "parameter instance \"%s\" (with value =%s) to iParameter<%s> "
            "instance.", spVal.target_type_info().name(),
            spVal.to_string().c_str(), typeid(ValueT).name() );
    }
    _set_value( p->value() );
}

template<typename ValueT> void
iParameter<ValueT>::_set_value( const ValueT & val ) {
    iSingularParameter::_set_is_set_flag();
    iTValue<ValueT>::_set_value(val);
}

/**@class Parameter
 * @brief User-side implementation class. Extension point for user parameters type.
 *
 * There is no default implementation of this class --- only standard C classes
 * are implemented in Goo library.
 */
template<typename ValueT>
class Parameter;  // Defailt implementation is empty.

template <typename T> class IntegralParameter;
template <typename T> class FloatingPointParameter;
template <typename T> class EnumParameter;
template <typename T> class PointerParameter;

# ifndef SWIG
template<typename T>
using InsertableParameter = typename
    std::conditional< std::is_arithmetic<T>::value,
        typename std::conditional< std::is_same<T, bool>::value,
            Parameter<bool>,
            typename std::conditional< std::is_integral<T>::value,
                IntegralParameter<T>,
                typename std::conditional< std::is_floating_point<T>::value,
                    FloatingPointParameter<T>,
                    Parameter<T>
                    >::type
            >::type
        >::type,
        typename std::conditional< std::is_enum<T>::value,
            EnumParameter<T>,
            typename std::conditional< std::is_pointer<T>::value,
                PointerParameter<T>,
                Parameter<T>
                >::type
            >::type
    >::type;
# endif

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
template<typename ValueT>
class Parameter<List<ValueT> > :
            public mixins::iDuplicable< iAbstractParameter,
                                        Parameter< List<ValueT> >,
                                        /*protected?*/ InsertableParameter< ValueT > > {
public:
    typedef mixins::iDuplicable<iAbstractParameter,
                                Parameter< List<ValueT> >,
                                InsertableParameter<ValueT> > DuplicableParent;
private:
    bool _setToDefault;
    List<ValueT> _values;
protected:
    virtual void _V_push_value( const ValueT & v ) {
        if( _setToDefault ) {
            _values.clear();
            _setToDefault = false;
        }
        _values.push_back( v ); }

    virtual void _V_parse_argument( const char * strval ) override {
        InsertableParameter<ValueT>::_V_parse_argument( strval );
        _V_push_value( InsertableParameter<ValueT>::value() ); }
public:
    const List<ValueT> & values() const { return _values; }

    template<class ... Types>
    Parameter( const std::initializer_list<ValueT> & il, Types ... args ) :
        DuplicableParent( args ... , *il.begin() ),
        _setToDefault( true ),
        _values( il )
    {
        this->_unset_singular();
        this->_set_is_set_flag();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() );
    }

    template<class ... Types>
    Parameter( const List<ValueT> & il, Types ... args ) :
        DuplicableParent( args ... , *il.begin() ),
        _setToDefault( true ),
        _values( il )
    {
        this->_unset_singular();
        this->_set_is_set_flag();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() );
    }

    template<class ... Types>
    Parameter( Types ... args ) :
        DuplicableParent( args ... ),
        _setToDefault( false )
    {   this->_unset_singular();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() );
    }

    Parameter( const Parameter<List<ValueT> > & orig ) :
        DuplicableParent( orig ),
        _setToDefault( orig._setToDefault ),
        _values( orig._values )
    {}

    ~Parameter() {}

    /// Returns `_setToDefault` flag.
    bool is_set_to_default() const { return _setToDefault; }

    /// Sets `_setToDefault` flag. See note about this flag in class
    /// description.
    void set_to_default(bool v) { _setToDefault = v; }

    /// This method is usually used by some user code desiring set all the
    /// parameters list at once. It should not be used by Goo API, during the
    /// normal argument parsing cycle.
    void assign( const List<ValueT> & plst ) {
        for( auto v : plst ) {
            _V_push_value(v);
        }
    }

    friend class DictInsertionProxy;

    using iAbstractParameter::name;
    using iAbstractParameter::description;
    using iAbstractParameter::shortcut;
    // ... whatever?
};

template<typename T> const List<T> &
iBaseValue::as_array_of() const {
    typedef Parameter<List<T> > const * CastTarget;
    auto ptr = dynamic_cast<CastTarget>(this);
    if( !ptr ) {
        const iSingularParameter * namedP =
                                dynamic_cast<const iSingularParameter *>(this);
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
    }
    return ptr->values();
}

template<typename T> const T &
iBaseValue::as() const {
    typedef iParameter<T> const * CastTarget;
    auto ptr = dynamic_cast<CastTarget>(this);
    if( !ptr ) {
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
    }
    return ptr->value();
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

