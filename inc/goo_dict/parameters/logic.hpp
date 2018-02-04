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
# include "goo_dict/common_aspects.hpp"

namespace goo {
namespace dict {

/**@brief Logic option argument type.
 *
 * Most common usage of this object is to provide logical flag for
 * enabling/disabling some functionality. E.g.:
 *      $ do_something -v
 * Or, with equivalent meaning:
 *      $ do_something --verbose=true
 * Besides of true/false values, the following synonyms are accepted:
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
template<typename ... AspectTs>
class Parameter<bool, AspectTs...> : public mixins::iDuplicable< iAbstractValue
                                                             , Parameter<bool, AspectTs...>
                                                             , TValue<bool, AspectTs...> > {
public:
    typedef mixins::iDuplicable< iAbstractValue
                               , Parameter<bool, AspectTs...>
                               , TValue<bool, AspectTs...>
                               > DuplicableParent;
    template<typename ... Ts> Parameter( Ts ... ctrArgs) : DuplicableParent( ctrArgs... ) {}
    // ...
};

namespace aspects {
template<>
struct iStringConvertible::ConversionTraits<bool> {
    typedef bool Value;

    /// Sets parameter value from string. Following strings are acceptable
    /// with appropriate meaning (case insensitive):
    /// (true|enable|on|yes|1)    logically corresponds to `true';
    /// (false|disable|off|no|0)  logically corresponds to `false'.
    static Value parse_string_expression(const char *stv);

    /// Returns 'True' or 'False' depending on current value.
    static std::string to_string_expression(const Value &v);
};
}  // namespace aspects

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_LOGICAL_H

