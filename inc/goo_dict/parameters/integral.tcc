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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_INTEGRAL_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_INTEGRAL_H

# include "goo_dict/parameter.tcc"
# include <climits>

namespace goo {
namespace dict {

template<typename T> T
integral_safe_parse( const char * str, int base=0 ) {
    /// This implementation is used for any type shorter than long int or for
    /// signed long int. The unsigned long int and long long integral types
    /// have to be parsed by specialized functions.
    char * end;
    long int r = strtol( str, &end, base );
    if( '\0' == *str ) {
        emraise( badParameter, "Unable to parse empty string as integral number "
            "with base %d.", base );
    }
    if( ERANGE == errno && LONG_MIN == r ) {
        emraise( underflow, "Given string token \"%s\" represents an integer "
            "number which is lesser than upper limit of signed long integer.",
            str );
    }
    if( ERANGE == errno && LONG_MAX == r ) {
        emraise( overflow, "Given string token \"%s\" represents an integer "
            "number which is greater than upper limit of signed long integer.",
            str );
    }
    if( *end != '\0' ) {
        emraise( badParameter, "Unable to parse token %s as integral number "
            "with base %d. Extra symbols on tail: %s.", str, base, end );
    }
    if( r < std::numeric_limits<T>::min() ) {
        emraise( underflow, "Given string token \"%s\" represents an integer "
            "number which is lesser than upper limit of%s integer type of "
            "length %d.", str, (std::numeric_limits<T>::is_signed() ?
                            " signed" : "unsigned" ), (int) sizeof(T) );
    }
    if( r > std::numeric_limits<T>::max() ) {
        emraise( overflow, "Given string token \"%s\" represents an integer "
            "number which is greater than upper limit of%s integer type of "
            "length %d.", str, (std::numeric_limits<T>::is_signed() ?
                            " signed" : "unsigned" ), (int) sizeof(T) );
    }
    return r;
}

template<> unsigned long
integral_safe_parse<unsigned long>( const char * str, int base );

# ifdef TYPES_128BIT_LENGTH
template<> long long int
integral_safe_parse<long long int>( const char * str, int base );

template<> unsigned long long int
integral_safe_parse<unsigned long long int>( const char * str, int base );
# endif


template<typename T>
class IntegralParameter : public mixins::iDuplicable<
                                    iAbstractParameter,
                                    IntegralParameter<T>,
                                    iParameter<T> > {
public:
    typedef T Integral;
    typedef mixins::iDuplicable< iAbstractParameter,
                                 IntegralParameter<T>,
                                 iParameter<T> > DuplicableParent;

    /// Only long option ctr.
    IntegralParameter(  const char * name_,
                        const char * description_ );
    /// Long option with shortcut.
    IntegralParameter(  char shortcut_,
                        const char * name_,
                        const char * description_ );
    /// Only short option.
    IntegralParameter(  char shortcut_,
                        const char * description_ );
    /// Only long option ctr.
    IntegralParameter(  const char * name_,
                        const char * description_,
                        Integral default_ );
    /// Long option with shortcut.
    IntegralParameter(  char shortcut_,
                        const char * name_,
                        const char * description_,
                        Integral default_ );
    /// Only short option.
    IntegralParameter(  char shortcut_,
                        const char * description_,
                        Integral default_ );

    IntegralParameter( const IntegralParameter<Integral> & o ) : DuplicableParent( o ) {}

protected:
    /// Internally uses strtol() from standard library.
    virtual Integral _V_parse( const char * ) const override;

    /// Returns 'True' or 'False' depending on current value.
    virtual std::string _V_stringify_value( const Integral & ) const override;
};


template<typename T>
IntegralParameter<T>::IntegralParameter( const char * name_,
                                         const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                            ) {}

template<typename T>
IntegralParameter<T>::IntegralParameter(  char shortcut_,
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
IntegralParameter<T>::IntegralParameter(  char shortcut_,
                                            const char * description_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular,
                              shortcut_
                            ) {}

template<typename T>
IntegralParameter<T>::IntegralParameter( const char * name_,
                                           const char * description_,
                                           Integral default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              '\0',
                              default_
                            ) {
}

template<typename T>
IntegralParameter<T>::IntegralParameter( char shortcut_,
                                           const char * name_,
                                           const char * description_,
                                           Integral default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              shortcut_,
                              default_
                            ) {}

template<typename T>
IntegralParameter<T>::IntegralParameter( char shortcut_,
                                           const char * description_,
                                           Integral default_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              shortcut_,
                              default_
                            ) {}

template<typename T> T
IntegralParameter<T>::_V_parse( const char * str ) const {
    _TODO_ // TODO
}

template<typename T> std::string
IntegralParameter<T>::_V_stringify_value( const Integral & ) const {
    _TODO_ // TODO
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_INTEGRAL_H

