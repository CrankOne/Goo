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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_LOGICAL_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_LOGICAL_H

# include "goo_dict/parameter.tcc"

namespace goo {
namespace dict {

/**@brief Logic option argument type.
 *
 * Most common usage of this object is to provide logical flag for
 * enabling/disabling some functionality. E.g.:
 *      $ do_something -v
 * Or, with equivalent meaning:
 *      $ do_something --verbose=true
 * Besides of true/false values, the following synonims are accepted:
 *      true : True, TRUE, yes, Yes, YES, enable, Enable, ENABLE, on, On, ON
 *      false: False, FALSE, no, No, NO, disable, Disable, DISABLE, off, Off, OFF
 * 
 * Note, that flag-option can not be required.
 * Possible construction variants:
 *      .p<bool>( 'v', "Enables verbose output" )
 *      .p<bool>( 'q', "Be quiet", true )
 *      .p<bool>( "quet", "Be quiet", true )
 *      .p<bool>( "verbose", "Enables verbose output" )
 *      .p<bool>( 'v', "verbose", "Enables verbose output" )
 *      .p<bool>( 'q', "quet", "Be quiet", true )
 * If there are default argument specified, the argument for such kind of an option
 * is mandatory. Otherwise, it must be omitted. E.g.:
 *   o to enable `.p<bool>( 'v', "Enables verbose output" )` it is enough to:
 *      $ myprogram -v
 *   o contraversary, for .p<bool>( 'q', "Be quiet", true ) it is necessary to:
 *      $ myprogram -q false
 * This aspect affects generated help message.
 * */
template<>
class Parameter<bool> : public mixins::iDuplicable< iSingularParameter
                                                 , Parameter<bool>
                                                 , iParameter<bool> > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
protected:
    /// Insertion proxy has access to protected members and can invoke this
    /// method to make the parameter a flag;
    void reset_flag() {
        _set_is_flag_flag();
        _set_value(false);
    }
public:
    //using Parameter<bool>::Value;

    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_ );
    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_ );
    /// Only short option.
    Parameter( char shortcut_,
               const char * description_ );
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               bool default_ );
    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               bool default_ );
    /// Only short option.
    Parameter( char shortcut_,
               const char * description_,
               bool default_ );

    // This is to prevent implicit conversion from char * to bool --- fails at compile time.
    Parameter( const char *,
               const char *,
               const char * );
    // This is to prevent implicit conversion from char * to bool --- fails at compile time.
    Parameter( char,
               const char *,
               const char *,
               const char * );

    Parameter( const Parameter<bool> & o ) : DuplicableParent( o ) {}

    /// This method is used mostly by Configuration class.
    virtual void set_option( bool );

    friend class ::goo::dict::DictInsertionProxy;
# if 0
protected:
    /// Sets parameter value from string. Following strings are acceptable
    /// with appropriate meaning (case insensitive):
    /// (true|enable|on|yes|1)    logically corresponds to `true';
    /// (false|disable|off|no|0)  logically corresponds to `false'.
    virtual Value _V_parse( const char * ) const override;

    /// Returns 'True' or 'False' depending on current value.
    virtual std::string _V_stringify_value( const Value & ) const override;
# endif
};

template<>
struct iStringConvertibleParameter::ConversionTraits<bool> {
    typedef bool Value;
    static Value parse_string_expression( const char * stv );
    static std::string to_string_expression( const Value & v );
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_LOGICAL_H

