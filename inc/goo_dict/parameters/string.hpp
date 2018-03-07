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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_STRING_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_STRING_H

# include "goo_dict/parameter.tcc"
# include "goo_dict/common_aspects.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) \
  && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

namespace goo {
namespace dict {

/**@brief String parameter.
 *
 * The string parameter keeps given token as is. I.e., for string parameter `a':
 *      $ do_something -a one
 * Will cause parameter `a` to be set to string value one.
 *
 * Possible construction variants:
 *      .p<std::string>( "str-parameter",
 *                       "A string parameter referenced by long option" )
 *      .p<std::string>( 'a', "str-parameter-1",
 *                       "A string parameter referenced by long option and "
 *                       "shortcut" )
 *      .p<std::string>( 'b', "str-parameter-2", "default value",
 *                       "A string parameter wtih default valuer eferenced by "
 *                       "long option and shortcut" )
 *      .p<std::string>( "str-parameter-3", "default value",
 *                       "A string parameter wtih default value referenced by "
 *                       "long option only" )
 *      ;
 * In order to avoid collisions, use empty string ("") or nullptr instead of
 * name argument when only short option is given.
 * */
template<typename ... AspectTs>
class Parameter<std::string, AspectTs...> : public mixins::iDuplicable< iAbstractValue
                                                                      , Parameter<std::string, AspectTs... >
                                                                      , TValue<std::string, AspectTs...>
                                                                      , AbstractValueAllocator & > {
public:
    typedef mixins::iDuplicable< iAbstractValue
                               , Parameter<std::string, AspectTs... >
                               , TValue<std::string, AspectTs...>
                               , AbstractValueAllocator & > DuplicableParent;
public:
    Parameter( const Parameter<std::string> & o ) : DuplicableParent( o ) {}
    template<typename ... Ts> Parameter( Ts ... ctrArgs ) : DuplicableParent(ctrArgs...) {}
};

namespace aspects {
template<>
struct iStringConvertible::ConversionTraits<std::string> {
    typedef std::string Value;

    static std::string parse_string_expression(const char *stv);

    static std::string to_string_expression(const std::string &v);
};
}  // namespace aspects

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_STRING_H


