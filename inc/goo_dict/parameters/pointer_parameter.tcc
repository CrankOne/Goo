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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PTR_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PTR_H

# include "goo_dict/parameter.tcc"

# if !defined(_Goo_m_DISABLE_DICTIONARIES) \
  && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)

namespace goo {
namespace dict {

/**@brief Generic pointer parameter.
 * */
template<typename T>
class PointerParameter : public mixins::iDuplicable< iAbstractValue
                                                 , PointerParameter<T>
                                                 , iParameter<T>
                                                 , AbstractValueAllocator & > {
public:
    typedef T PointerType;
    typedef typename std::remove_pointer<T>::type DataType;
    typedef mixins::iDuplicable< iAbstractValue
                               , PointerParameter<T>
                               , iParameter<T>
                               , AbstractValueAllocator & > DuplicableParent;
public:
    /// Long option with shortcut.
    PointerParameter( char shortcut_,
                    const char * name_,
                    const char * description_ ) :
                        DuplicableParent( (name_ ? ('\0' == name_[0] ?
                                                    nullptr : name_) : nullptr),
                                          description_,
                              0x0 | AbstractParameter::atomic
                                  | AbstractParameter::singular
                                  | AbstractParameter::shortened,
                              shortcut_
                            ) {
    }

    /// Only long option ctr.
    PointerParameter( const char * name_,
                    const char * description_ ) :
                    PointerParameter( '\0', name_, description_ ) {}

    /// Only shortcut option ctr.
    PointerParameter( char shortcut_,
                    const char * description_ ) :
                    PointerParameter( shortcut_, nullptr, description_ ) {}

    PointerParameter( const PointerParameter<T> & o ) : DuplicableParent( o ) {}

    friend class ::goo::dict::DictInsertionProxy;
# if 0
protected:
    /// Sets parameter value from given string.
    virtual PointerType _V_parse( const char * strval ) const override
        { emraise( badArchitect, "Attempt to parse pointer parameter." ); }

    /// Returns set value.
    virtual std::string _V_stringify_value( const T & ptr ) const override {
        char bf[32];
        snprintf( bf, sizeof(bf), "%p", ptr );
        return bf;
    }
# endif
};

}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES) && !defined(_Goo_m_DISABLE_APP_CONF_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_PTR_H

