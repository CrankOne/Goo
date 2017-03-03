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

namespace goo {
namespace dict {

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
IntegralParameter<T>::_V_parse( const char * ) const {
    _TODO_ // TODO
}

template<typename T> std::string
IntegralParameter<T>::_V_stringify_value( const Integral & ) const {
    _TODO_ // TODO
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_INTEGRAL_H

