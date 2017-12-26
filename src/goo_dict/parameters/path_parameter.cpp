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

# include "goo_dict/parameters/path_parameter.hpp"

namespace goo {
namespace dict {

using filesystem::Path;

Parameter<Path>::Parameter( char shortcut_,
                            const char * name_,
                            const char * description_,
                            const char * default_,
                            Path::Interpolator * ip ) :
                        DuplicableParent( (name_ ? ('\0' == name_[0] ?
                                                nullptr : name_) : nullptr),
                                          description_,
                              0x0 | iAbstractParameter::atomic
                                  | iAbstractParameter::singular
                                  | iAbstractParameter::shortened,
                              shortcut_
                            ) {
    if( default_ && '\0' != default_[0] ) {
        _set_value( default_ );
    }
    if( ip ) {
        _mutable_value().interpolator( ip );
    }
}

Parameter<Path>::Parameter( const char * name_,
                                   const char * description_,
                                   const char * default_,
                                   Path::Interpolator * ip ) :
                        DuplicableParent( name_,
                              description_,
                              0x0 | iAbstractParameter::set
                                  | iAbstractParameter::atomic
                                  | iAbstractParameter::singular,
                              '\0' ) {
    if( default_ && '\0' != default_[0] ) {
        _set_value( default_ );
    }
    if( ip ) {
        _mutable_value().interpolator( ip );
    }
}

filesystem::Path
iStringConvertibleParameter::ConversionTraits<filesystem::Path>::parse_string_expression( const char * stv ) {
    return stv;
}

std::string
iStringConvertibleParameter::ConversionTraits<filesystem::Path>::to_string_expression( const Value & val ) {
    return val;
}

}  // namespace dict
}  // namespace goo


