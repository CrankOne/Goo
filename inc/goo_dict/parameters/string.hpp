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
template<>
class Parameter<std::string> : public mixins::iDuplicable< iBaseValue,
                                                           Parameter<std::string>,
                                                           iParameter<std::string> > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               const char * default_=nullptr );

    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               const char * default_=nullptr );

    Parameter( const char * name_,
               const char * description_,
               const std::string & default_ ) :
            Parameter( name_, description_, default_.c_str() ) {}

    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               const std::string & default_ ) :
            Parameter( shortcut_, name_, description_, default_.c_str() ) {}

    Parameter( const Parameter<std::string> & o ) : DuplicableParent( o ) {}

    friend class ::goo::dict::DictInsertionProxy;
};

template<>
struct iStringConvertibleParameter::ConversionTraits<std::string> {
    typedef std::string Value;
    static std::string parse_string_expression( const char * stv );
    static std::string to_string_expression( const std::string & v );
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_STRING_H


