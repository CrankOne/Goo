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

# ifndef H_GOO_PARAMETERS_COMMON_ASPECTS_H
# define H_GOO_PARAMETERS_COMMON_ASPECTS_H

# include "goo_types.h"
# include "goo_sys/exception.hpp"

# include "goo_dict/value.hpp"
# include "goo_dict/parameter.tcc"

# include <tuple>

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

namespace goo {
namespace dict {
namespace aspects {

/// Defines presence of the the textual attribute that should contain
/// human-readable description for parameter or dictionary.
class Description {
private:
     String _d;
public:
    explicit Description( const char * t
                        , TheAllocatorHandle<char> ah ) : _d(t, ah) {}
    virtual const String & description() { return _d; }
    virtual void description( const String & t ) { _d = t; }
};

/// Defines the "is required" and "requires argument" flags for parameters.
/// The "requires argument"=false state is usually possible for command-line
/// arguments --- POSIX standard defines such flags. The "is required" flag
/// makes sense only with composition of the IsSet() aspect.
struct ProgramOption : public BoundMixin {
private:
    bool _isRequired
       , _expectsArgument
       , _mayBeImplicit
       ;
protected:
    /// Virtual function. Does nothing by default, but may be overriden by
    /// subclass (see ImplicitValue).
    virtual void _V_assign_implicit_value() {
        wprintf( "Default _V_set_implicit_value() has no side effects.\n" );
    }
public:
    explicit ProgramOption( bool isRequired=false
                          , bool expectsArgument=true
                          , bool mayBeImplicitly=false) : _isRequired(isRequired)
                                                        , _expectsArgument(expectsArgument)
                                                        , _mayBeImplicit(mayBeImplicitly) {}
    bool is_required() const { return _isRequired; }
    virtual void set_required(bool v) { _isRequired = true; }
    /// Returns whether this parameter expects an argument (otherwise, being a flag).
    bool expects_argument() const { return _expectsArgument; }
    /// (Un)sets the argument indicating this parameter is expecting argument.
    virtual void set_expects_argument(bool v) { _expectsArgument = v; }
    bool may_be_set_implicitly() const { return _mayBeImplicit; }
    virtual void set_being_implicit( bool v ) { _mayBeImplicit = v; }
    virtual void assign_implicit_value() { _V_assign_implicit_value(); }
};

/// Defines "is set" flag for parameters.
struct IsSet {
private:
    bool _isSet;
public:
    explicit IsSet( bool v ) : _isSet(v) {}
    IsSet() : _isSet(false) {}
    virtual bool is_set() const { return _isSet; }
    virtual void set_is_set(bool v) { _isSet = true; }

    int after_set() { _isSet = true; return 0; }
};

/// Defines "shortcut" attribute, used for application configuration
/// dictionaries.
struct CharShortcut {
private:
    char _shortcut;
public:
    explicit CharShortcut(char c) : _shortcut(c) {}
    CharShortcut( ) : _shortcut('\0') {}
    char shortcut( ) const { return _shortcut; }
    virtual char shortcut( char c_ ) {
        char c = _shortcut;
        _shortcut = c_;
        return c;
    }
};

/// Array aspect.
/// TODO: append/wipe-on-set modifiers.
struct Array {
private:
    bool _isArray;
public:
    explicit Array(bool isArray) : _isArray(isArray) {}
    virtual bool is_array() const { return _isArray; }
};

/**@class iStringConvertibleParameter
 * @brief Generic interface for parameter instance that may be converted to str.
 *
 * This is a data type-agnostic part of the string-convertible parameters.
 */
class iStringConvertible : public BoundMixin {
protected:
    /// Shall translate string expression to C++ value.
    virtual void _V_parse_argument( const char * ) = 0;
    /// Shall render C++ value to string.
    virtual String _V_to_string( ) const = 0;
public:
    /// Translates value from string representation.
    void parse_argument( const char * strval ) { _V_parse_argument( strval ); }
    /// Renders string from C++ value.
    String to_string() const { return _V_to_string(); }
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

/**@brief Default template implementing value parameters that may be converted
 * to string.
 *
 * One may be interested in partial specialization of this class to support
 * various formatting modifiers.
 * */
template< typename ValueT
        , typename ... AspectTs>
class TStringConvertible : public iStringConvertible {
public:
    typedef ValueT Value;
    typedef typename iStringConvertible::ConversionTraits<Value> ValueTraits;
    typedef TStringConvertible<ValueT> Self;
    typedef TValue<ValueT, AspectTs...> TypedSelf;
protected:
    /// Sets the value kept by current instance from string expression.
    virtual void _V_parse_argument( const char *strval ) override {
        static_cast<TypedSelf&>(BoundMixin::target())
                .value(ValueTraits::parse_string_expression(strval));
    }

    /// Expresses the value kept by current instance as a string.
    virtual String _V_to_string() const override {
        return ValueTraits::to_string_expression(
                static_cast<const TypedSelf&>(BoundMixin::target()).value());
    }
public:
    TStringConvertible() {}
    explicit TStringConvertible(const ValueT &v) : TValue<Value>(v) {}
    explicit TStringConvertible(const Self &o) : TValue<Value>(o) {}
};


template< typename ValueT
        , typename ... AspectTs>
class TStringConvertible<dict::Array<ValueT>, AspectTs...> : public iStringConvertible {
public:
    typedef ValueT Value;
    typedef typename iStringConvertible::ConversionTraits<Value> ValueTraits;
    typedef TStringConvertible<dict::Array<ValueT>, AspectTs...> Self;
    typedef Parameter<dict::Array<ValueT>, AspectTs...> TypedSelf;
protected:
    /// Sets the value kept by current instance from string expression.
    virtual void _V_parse_argument( const char *strval ) override {
        static_cast<TypedSelf&>(BoundMixin::target())
                .append(ValueTraits::parse_string_expression(strval));
    }

    /// Expresses the value kept by current instance as a string.
    virtual std::string _V_to_string() const override {
        // TODO stringify array of objects
        return ValueTraits::to_string_expression(
                static_cast<const TypedSelf&>(BoundMixin::target()).value());
    }
public:
    TStringConvertible() {}
    explicit TStringConvertible(const ValueT &v) : TValue<Value>(v) {}
    explicit TStringConvertible(const Self &o) : TValue<Value>(o) {}
};


template< typename ValueT
        , typename ... AspectTs>
class ImplicitValue : public ProgramOption {
public:
    typedef TValue<ValueT, AspectTs...> TypedSelf;
private:
    ValueT _implicitValue;
protected:
    virtual void _V_assign_implicit_value() override {
        static_cast<TypedSelf&>(BoundMixin::target())
                .value(_implicitValue);
        //static_cast<TValue<ValueT, AspectTs...>*>(this)->value(_implicitValue);
    }
public:
    ImplicitValue( bool isRequired=false
                 , bool requiresValue=true
                 , bool mayBeImplicit=false) : ProgramOption( isRequired
                                                      , requiresValue
                                                      , mayBeImplicit ) {}
    virtual void set_implicit_value( const ValueT & v ) {
        set_being_implicit( true );
        _implicitValue = v;
    }
};

}  // namespace aspects


}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_COMMON_ASPECTS_H
