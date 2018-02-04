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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_FLOATING_POINT_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_FLOATING_POINT_H

// TODO: This class has to be re-designed. The preleminary range checks has to
// be performed on _set_value, not just within safe_parse procedure.

# include "goo_dict/parameter.tcc"
# include "goo_dict/common_aspects.hpp"

namespace goo {
namespace dict {

template<typename T> T _floating_point_parse( const char *, char ** );

template<typename T> T
floating_point_safe_parse( const char * str ) {
    char * end;
    T r = _floating_point_parse<T>( str, &end );
    if( '\0' == *str ) {
        emraise( parserFailure, "Unable to parse empty string as floating "
            "point number." );
    }
    if( ERANGE == errno && 0 == r ) {
        emraise( underflow, "Given string \"%s\" represents a floating "
            "point number which will cause underflow of requested floating "
            "point type.", str );
    }
    if( ERANGE == errno && (HUGE_VAL == r || -HUGE_VAL == r) ) {
        emraise( overflow, "Given string \"%s\" represents a floating "
            "point number which will cause overflow of requested floating "
            "point type.", str );
    }
    if( *end != '\0' ) {
        emraise( parserFailure, "Unable to parse expression %s as floating "
            "point number. Extra symbols on tail: \"%s\".", str, end );
    }
    return r;
}

template<> float _floating_point_parse( const char *, char ** );
template<> double _floating_point_parse( const char *, char ** );
template<> long double _floating_point_parse( const char *, char ** );
// template<> __float128 _floating_point_safe_parse( const char * ); // TODO?


template< typename T
        , typename ... AspectTs>
class FloatingPointParameter : public mixins::iDuplicable< iAbstractValue
                                                       , FloatingPointParameter<T, AspectTs ...>
                                                       , Parameter<T, AspectTs...> > {
public:
    typedef T Float;
    typedef mixins::iDuplicable< iAbstractValue
                               , FloatingPointParameter<T, AspectTs ...>
                               , Parameter<T, AspectTs...> > DuplicableParent;

    template<typename ... Ts> FloatingPointParameter( Ts ... ctrArgs ) : DuplicableParent(ctrArgs...) {}
    //FloatingPointParameter( const FloatingPointParameter<Float> & o ) : DuplicableParent( o ) {}
};

namespace aspects {
template<typename T>
struct iStringConvertible::ConversionTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    typedef T Value;

    static Value parse_string_expression(const char *stv) { return floating_point_safe_parse<T>(stv); }

    static std::string to_string_expression(const Value &v) {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
};
}  // namespace aspects

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_FLOATING_POINT_H

