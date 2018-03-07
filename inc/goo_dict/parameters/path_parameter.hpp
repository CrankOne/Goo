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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PATH_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PATH_H

# include "goo_dict/parameter.tcc"
# include "goo_path.hpp"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) \
  && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

namespace goo {
namespace dict {

// TODO

# if 0
/**@brief Path parameter.
 *
 * TODO: doc
 * */
template<>
class Parameter<filesystem::Path> :
        public mixins::iDuplicable< iAbstractValue,
                                    Parameter<filesystem::Path>,
                                    iParameter<filesystem::Path>
                                    # ifdef SWIG  // see iss#163
                                    , false
                                    # endif  // SWIG
                                    > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               const char * default_=nullptr,
               filesystem::Path::Interpolator * ip=nullptr );

    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               const char * default_=nullptr,
               filesystem::Path::Interpolator * ip=nullptr );

    Parameter( const char * name_,
               const char * description_,
               const std::string & default_,
               filesystem::Path::Interpolator * ip=nullptr ) :
            Parameter( name_, description_, default_.c_str(), ip ) {}

    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               const std::string & default_,
               filesystem::Path::Interpolator * ip=nullptr ) :
            Parameter( shortcut_, name_, description_, default_.c_str(), ip ) {}

    Parameter( const Parameter<filesystem::Path> & o ) : DuplicableParent( o ) {}

    friend class ::goo::dict::DictInsertionProxy;
};


template<>
struct iStringConvertibleParameter::ConversionTraits<filesystem::Path> {
    typedef filesystem::Path Value;
    static Value parse_string_expression( const char * stv );
    static std::string to_string_expression( const Value & v );
};
# endif

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PATH_H

