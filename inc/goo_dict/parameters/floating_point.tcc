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

# include "goo_dict/parameter.tcc"

namespace goo {
namespace dict {

template<typename T> T _floating_point_parse( const char *, char ** );

template<typename T> T
floating_point_safe_parse( const char * str ) {
    char * end;
    T r = _floating_point_parse<T>( str, &end );
    if( '\0' == *str ) {
        emraise( badParameter, "Unable to parse empty string as floating "
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
        emraise( badParameter, "Unable to parse expression %s as floating "
            "point number. Extra symbols on tail: \"%s\".", str, end );
    }
    return r;
}

template<> float _floating_point_parse( const char *, char ** );
template<> double _floating_point_parse( const char *, char ** );
template<> long double _floating_point_parse( const char *, char ** );
// template<> __float128 _floating_point_safe_parse( const char * ); // TODO?


template<typename T>
class FloatingPointParameter : public mixins::iDuplicable<
                                    iAbstractParameter,
                                    FloatingPointParameter<T>,
                                    iParameter<T> > {
public:
    typedef T Float;
    typedef mixins::iDuplicable< iAbstractParameter,
                                 FloatingPointParameter<T>,
                                 iParameter<T> > DuplicableParent;

    /// Only long option ctr.
    FloatingPointParameter(  const char * name_,
                        const char * description_ );
    /// Long option with shortcut.
    FloatingPointParameter(  char shortcut_,
                        const char * name_,
                        const char * description_ );
    /// Only short option.
    FloatingPointParameter(  char shortcut_,
                        const char * description_ );
    /// Only long option ctr.
    FloatingPointParameter(  const char * name_,
                        const char * description_,
                        Float default_ );
    /// Long option with shortcut.
    FloatingPointParameter(  char shortcut_,
                        const char * name_,
                        const char * description_,
                        Float default_ );
    /// Only short option.
    FloatingPointParameter(  char shortcut_,
                        const char * description_,
                        Float default_ );

    FloatingPointParameter( const FloatingPointParameter<Float> & o ) : DuplicableParent( o ) {}

protected:
    /// Internally uses strtol() from standard library.
    virtual Float _V_parse( const char * ) const override;

    /// Returns 'True' or 'False' depending on current value.
    virtual std::string _V_stringify_value( const Float & ) const override;
};


template<typename T>
FloatingPointParameter<T>::FloatingPointParameter( const char * name_,
                                         const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                            ) {}

template<typename T>
FloatingPointParameter<T>::FloatingPointParameter(  char shortcut_,
                                          const char * name_,
                                          const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::shortened,
                              shortcut_
                            ) {}

template<typename T>
FloatingPointParameter<T>::FloatingPointParameter( char shortcut_,
                                         const char * description_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular,
                              shortcut_
                            ) {}

template<typename T>
FloatingPointParameter<T>::FloatingPointParameter( const char * name_,
                                         const char * description_,
                                         Float default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              '\0',
                              default_
                            ) {}

template<typename T>
FloatingPointParameter<T>::FloatingPointParameter( char shortcut_,
                                           const char * name_,
                                           const char * description_,
                                           Float default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set
                                | iAbstractParameter::shortened,
                              shortcut_,
                              default_
                            ) {}

template<typename T>
FloatingPointParameter<T>::FloatingPointParameter( char shortcut_,
                                           const char * description_,
                                           Float default_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set
                                | iAbstractParameter::shortened,
                              shortcut_,
                              default_
                            ) {}

template<typename T> T
FloatingPointParameter<T>::_V_parse( const char * str ) const {
    return floating_point_safe_parse<T>(str);
}

template<typename T> std::string
FloatingPointParameter<T>::_V_stringify_value( const Float & v ) const {
    std::stringstream ss;
    ss << v;  // TODO: scientific, at least
    return ss.str();
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_FLOATING_POINT_H

