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

# ifndef H_GOO_PARAMETERS_DICTIONARY_PARAMETER_ENUM_H
# define H_GOO_PARAMETERS_DICTIONARY_PARAMETER_ENUM_H

# include "goo_dict/parameter.tcc"
# include "goo_path.hpp"

namespace goo {

namespace aux {
template<typename EnumT, typename Enabled=void>
class Enum;  // undefined for EnumT=<non-enum>

template<typename EnumT>
class Enum<EnumT, typename std::enable_if<std::is_enum<EnumT>::value>::type> {
public:
    typedef std::unordered_map<std::string, EnumT> Entries;

private:
    EnumT _eValue;
    /// This static field has to be initialized with some compiler-related magic.
    static Entries * _entriesPtr;
public:
    Enum( EnumT v ) : _eValue(v) {}
    Enum() {}

    operator const EnumT&() const{ return _eValue; }

    static EnumT parse( const std::string & );
    static std::string to_string( EnumT );
    static void add_entry( const std::string &, EnumT val );
    static std::vector<std::string> available_values();
};  // class enum

/**@def GOO_ENUM_PARAMETER_DEFINE
 *
 * Auxilliary macro for run-time indexing of enum-to-str values for particular
 * type.
 *
 * Example: GOO_ENUM_PARAMETER_DEFINE( foo::Bar::Baz::one, one )
 */
# define GOO_ENUM_PARAMETER_DEFINE( entry, entryName )                          \
static void __goo_define_enum_entry_ ## entryName() __attribute__((__constructor__(155))); \
static void __goo_define_enum_entry_ ## entryName()  {                          \
    ::goo::aux::Enum<decltype(entry)>                                           \
        ::add_entry( STRINGIFY_MACRO_ARG(entryName), entry );                   \
}
}// namespace goo::aux

namespace dict {

/**@brief Enum parameter.
 *
 * This "enumeration" dictionary parameter introduces restrictionas for certain
 * string parameter.
 * */
template<typename EnumT>
class Parameter<aux::Enum<EnumT> > :
                        public mixins::iDuplicable< iAbstractParameter,
                                                    Parameter<aux::Enum<EnumT> >,
                                                    iParameter<aux::Enum<EnumT> > > {
public:
    typedef typename aux::Enum<EnumT> Value;
    typedef mixins::iDuplicable< iAbstractParameter,
                                 Parameter<aux::Enum<EnumT> >,
                                 iParameter<aux::Enum<EnumT> > > DuplicableParent;
    typedef iParameter<aux::Enum<EnumT> > exactIParameter;
public:
    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               EnumT default_ ) : DuplicableParent( (name_ ? ('\0' == name_[0] ?
                                                nullptr : name_) : nullptr),
                                          description_,
                              0x0 | iAbstractParameter::atomic
                                  | iAbstractParameter::singular
                                  | iAbstractParameter::shortened,
                              shortcut_
                            ) {
        exactIParameter::_set_value( default_ );
    }

    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               EnumT default_ ) : Parameter( '\0', name_, description_, default_ ) {}

    /// Only shortcut option ctr.
    Parameter( char shortcut_,
               const char * description_,
               EnumT default_ ) : Parameter( shortcut_, nullptr, description_, default_ ) {}

    Parameter( const Parameter<aux::Enum<EnumT> > & o ) : DuplicableParent( o ) {}

    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Value _V_parse( const char * strval ) const override
        { return aux::Enum<EnumT>::parse(strval); }

    /// Returns set value.
    virtual std::string _V_stringify_value( const Value & eVal ) const override
        { return aux::Enum<EnumT>::to_string(eVal); }
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_ENUM_H


