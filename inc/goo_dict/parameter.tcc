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
# include "goo_mixins.tcc"
# include "goo_vcopy.tcc"

# include <list>
# include <cassert>

namespace goo {
namespace dict {

class InsertionProxy;

template<typename ValueT>
class iParameter;

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
class iAbstractParameter : public mixins::iDuplicable<iAbstractParameter>/* {{{ */ {
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

    /// Sets the "required" flag marking a mandatory parameter.
    void _set_is_argument_required_flag();

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
public:
    virtual ~iAbstractParameter();

    /// Returns pointer to name string.
    const char * name() const;

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

    friend class ::goo::dict::InsertionProxy;
};  /*}}}*/ // class iAbstractParameter




/* @class iSingularParameter
 * @brief Generic command-line parameter features interface.
 *
 * At this level of inheritance the less-generic definition of
 * parameter dictionary entries is defined.
 *
 * @ingroup appParameters
 */
class iSingularParameter : public iAbstractParameter /*{{{*/ {
protected:
    virtual void _V_parse_argument( const char * ) = 0;
    virtual std::string _V_to_string() const = 0;
public:
    /// Returns single-char shortcut for this parameter.
    char shortcut() const { return _shortcut; }

    /// Parses argument from string representation.
    void parse_argument( const char * strval ) {  _V_parse_argument( strval ); }

    /// Forms a human-readable string to be displayed at logs.
    std::string to_string() const { return _V_to_string(); }

    iSingularParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0' );

    /// Getter method.
    template<typename T> const T &
    as() const {
        auto ptr = dynamic_cast<iParameter<T> const *>(this);
        if( !ptr ) {
            if( this->name() ) {
                emraise(badCast, "Couldn't cast parameter \"%s\" to specified type.", name() );
            } else {
                emraise(badCast, "Couldn't cast parameter '%c' to specified type.", shortcut() );
            }
        }
        return ptr->value();
    }

    /// Getter method for multiple parameters.
    template<typename T> const std::list<T> &
    as_list_of() const;
};  // }}} class iSingularParameter




/**@class iParameter
 * @brief A valued intermediate class representing dictionary entry with
 * value of certain type.
 *
 * Defines common value-operation routines.
 * */
template<typename ValueT>
class iParameter : public iSingularParameter /*{{{*/ {
public:
    //typedef iAbstractParameter::ParameterEntryFlag ParameterEntryFlag;
    typedef ValueT Value;
    //using iSingularParameter::ParameterEntryFlag;
    typedef iSingularParameter::ParameterEntryFlag ParameterEntryFlag;
private:
    Value _value;
protected:
    void _set_value( const ValueT & );
    virtual void _V_parse_argument( const char * strval ) override {
        _set_value( _V_parse( strval ) ); }
    virtual std::string _V_to_string( ) const override {
        assert( this->is_set() );
        return _V_stringify_value( value() ); }

    virtual Value _V_parse( const char * ) const = 0;
    virtual std::string _V_stringify_value( const Value & ) const = 0;
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

    const ValueT & value() const;
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
        _value(defaultValue) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const iParameter<ValueT> & o ) :
        iSingularParameter(o),
        _value(o._value) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT> const ValueT &
iParameter<ValueT>::value() const {
    if( !is_set() ) {
        emraise( uninitialized,
            "Option %s has not been set while its value required.",
            name());
    }
    return _value;
}

template<typename ValueT> void
iParameter<ValueT>::_set_value( const ValueT & val ) {
    this->_set_is_set_flag();
    _value = val;
}

/*}}}*/  // class iParameter implementation

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
        Parameter<T>
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
 */
template<typename ValueT>
class Parameter<std::list<ValueT> > :
            public mixins::iDuplicable< iAbstractParameter,
                                        Parameter< std::list<ValueT> >,
                                        /*protected?*/ InsertableParameter< ValueT > > {
public:
    typedef mixins::iDuplicable<iAbstractParameter,
                                Parameter< std::list<ValueT> >,
                                InsertableParameter<ValueT> > DuplicableParent;
private:
    bool _setToDefault;
    std::list<ValueT> _values;
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
    const std::list<ValueT> & values() const { return _values; }

    template<class ... Types>
    Parameter( const std::initializer_list<ValueT> & il, Types ... args ) :
        DuplicableParent( args ... , *il.begin() ),
        _setToDefault( true ),
        _values( il )
    {
        this->_unset_singular();
        this->_set_is_set_flag();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() ); }

    template<class ... Types>
    Parameter( Types ... args ) :
        DuplicableParent( args ... ),
        _setToDefault( false )
    {   this->_unset_singular();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() ); }

    Parameter( const Parameter<std::list<ValueT> > & orig ) :
        DuplicableParent( orig ),
        _setToDefault( orig._setToDefault ),
        _values( orig._values )
    {}

    ~Parameter() {}

    friend class InsertionProxy;

    using iAbstractParameter::name;
    using iAbstractParameter::description;
    using iAbstractParameter::shortcut;
    // ... whatever?
};

template<typename T> const std::list<T> &
iSingularParameter::as_list_of() const {
    auto ptr = dynamic_cast<Parameter<std::list<T> > const *>(this);
    if( !ptr ) {
        if( this->name() ) {
            emraise(badCast, "Couldn't cast parameter \"%s\" to "
                    "requested list type.", name() );
        } else {
            emraise(badCast, "Couldn't cast parameter '%c' to requested "
                    "list type.", shortcut() );
        }
    }
    return ptr->values();
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

