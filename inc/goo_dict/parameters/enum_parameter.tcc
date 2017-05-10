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
namespace dict {

/**@brief Enum parameter.
 *
 * This "enumeration" dictionary parameter introduces restrictionas for certain
 * string parameter.
 * */
template<typename EnumT>
class EnumParameter : public mixins::iDuplicable< iAbstractParameter,
                                                    EnumParameter<EnumT>,
                                                    iParameter<EnumT> > {
public:
    typedef EnumT Enum;
    typedef mixins::iDuplicable< iAbstractParameter,
                                 EnumParameter<Enum>,
                                 iParameter<Enum> > DuplicableParent;
    typedef std::unordered_map<std::string, Enum> Entries;
private:
    static Entries * _entriesPtr;
public:
    static Enum str_to_enum( const std::string & str );
    static std::string enum_to_str( Enum eVal );
    static void add_entry( const std::string & strVal, Enum eVal );
    static std::vector<std::string> available_values();
public:
    /// Long option with shortcut.
    EnumParameter( char shortcut_,
               const char * name_,
               const char * description_,
               Enum default_ ) : DuplicableParent( (name_ ? ('\0' == name_[0] ?
                                                nullptr : name_) : nullptr),
                                          description_,
                              0x0 | iAbstractParameter::atomic
                                  | iAbstractParameter::singular
                                  | iAbstractParameter::shortened,
                              shortcut_
                            ) {
        DuplicableParent::_set_value( default_ );
    }

    /// Only long option ctr.
    EnumParameter( const char * name_,
               const char * description_,
               Enum default_ ) : EnumParameter( '\0', name_, description_, default_ ) {}

    /// Only shortcut option ctr.
    EnumParameter( char shortcut_,
               const char * description_,
               Enum default_ ) : EnumParameter( shortcut_, nullptr, description_, default_ ) {}

    EnumParameter( const EnumParameter<Enum> & o ) : DuplicableParent( o ) {}

    operator const Enum&() const { return DuplicableParent::value(); }

    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Enum _V_parse( const char * strval ) const override
        { return str_to_enum(strval); }

    /// Returns set value.
    virtual std::string _V_stringify_value( const Enum & eVal ) const override
        { return enum_to_str(eVal); }
};


//
// Implementation

template<typename EnumT>
typename EnumParameter<EnumT>::Entries * EnumParameter<EnumT>::_entriesPtr = nullptr;

template<typename EnumT> EnumT
EnumParameter<EnumT>::str_to_enum( const std::string & str ) {
    if( !_entriesPtr ) {
        emraise( badArchitect, "String-to-enum mapping wasn't defined. "
            "Consider usage of GOO_ENUM_PARAMETER_DEFINE macro to fill "
            "mapping object for enum type \"%s\".",
            typeid(Enum).name() );
    }
    auto it = _entriesPtr->find( str );
    if( _entriesPtr->end() == it ) {
        emraise( notFound, "String \"%s\" does not match to any known "
            "entries of enumeration \"%s\".", str.c_str(), typeid(Enum).name() );
    }
    return it->second;
}


template<typename EnumT> std::string
EnumParameter<EnumT>::enum_to_str( EnumT eVal ) {
    if( !_entriesPtr ) {
        emraise( badArchitect, "String-to-enum mapping wasn't defined. "
            "Consider usage of GOO_ENUM_PARAMETER_DEFINE macro to fill "
            "mapping object for enum type \"%s\".",
            typeid(Enum).name() );
    }
    for( auto p : *_entriesPtr ) {
        if( p.second == eVal ) {
            return p.first;
        }
    }
    emraise( notFound, "Couldn't find match for value %d of enum type "
        "\"%s\".", (int) eVal, typeid(Enum).name() );
}

template<typename EnumT> void
EnumParameter<EnumT>::add_entry( const std::string & strVal, EnumT eVal ) {
    if( !_entriesPtr ) {
        _entriesPtr = new Entries();
    }
    auto ir = _entriesPtr->emplace( strVal, eVal );
    if( !ir.second ) {
        emraise( nonUniq, "Duplicate insertion of entry \"%s\" for enum type \"%s\".",
            strVal.c_str(), typeid(Enum).name() );
    }
}

template<typename EnumT> std::vector<std::string>
EnumParameter<EnumT>::available_values() {
    std::vector<std::string> ret;
    if( !_entriesPtr ) {
        emraise( badArchitect, "String-to-enum mapping wasn't defined. "
            "Consider usage of GOO_ENUM_PARAMETER_DEFINE macro to fill "
            "mapping object for enum type \"%s\".",
            typeid(Enum).name() );
    }
    for( auto p : *_entriesPtr ) {
        ret.push_back( p.first );
    }
    return ret;
}

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
    ::goo::dict::EnumParameter<decltype(entry)>                                 \
        ::add_entry( STRINGIFY_MACRO_ARG(entryName), entry );                   \
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_ENUM_H


