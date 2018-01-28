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

# include <type_traits>

namespace goo {
namespace dict {

/**@brief Enum parameter.
 *
 * This "enumeration" dictionary parameter introduces restrictions for certain
 * string parameter. This class is designed for support the "classic" C++ enums
 * which permits quite restricted functionality. It is convinient, however, to
 * use them for short static enumerations defined at compile time.
 * */
template< typename EnumT
        , typename ... AspectTs>
class EnumParameter : public mixins::iDuplicable< iBaseValue<AspectTs...>
                                              , EnumParameter<EnumT, AspectTs...>
                                              , Parameter<EnumT, AspectTs...> > {
public:
    typedef EnumT Enum;
    typedef mixins::iDuplicable< iBaseValue<AspectTs...>,
                                 EnumParameter<Enum, AspectTs...>,
                                 Parameter<Enum, AspectTs...> > DuplicableParent;
    typedef std::unordered_map<std::string, Enum> Entries;
private:
    static Entries * _entriesPtr;
    static void _assert_entries_initialized();
public:
    static Enum str_to_enum( const std::string & str );
    static std::string enum_to_str( Enum eVal );
    static void add_entry( const std::string & strVal, Enum eVal );
    static std::vector<std::string> available_values();
public:
    EnumParameter( const EnumParameter<Enum> & o ) : DuplicableParent( o ) {}
    template<typename ... Ts> EnumParameter( Ts ... ctrArgs ) : DuplicableParent(ctrArgs...) {}

    operator const Enum&() const { return DuplicableParent::value(); }

};

namespace aspects {
template< typename EnumT >
struct iStringConvertible::ConversionTraits<EnumT, typename std::enable_if<std::is_enum<EnumT>::value>::type> {
    typedef EnumT Value;

    static Value parse_string_expression( const char *stv ) { return EnumParameter<Value>::str_to_enum( stv ); }

    static std::string to_string_expression( const Value &v ) { return EnumParameter<Value>::enum_to_str( v ); }
};
}


//
// Implementation

template<typename EnumT, typename ... AspectTs>
typename EnumParameter<EnumT, AspectTs...>::Entries *
        EnumParameter<EnumT, AspectTs...>::_entriesPtr = nullptr;

template<typename EnumT, typename ... AspectTs> void
EnumParameter<EnumT, AspectTs...>::_assert_entries_initialized() {
    if( !_entriesPtr ) {
        emraise( badArchitect, "String-to-enum mapping wasn't defined. "
            "Consider usage of GOO_ENUM_PARAMETER_DEFINE macro to fill "
            "mapping object for enum type \"%s\".",
            typeid(Enum).name() );
    }
}

template<typename EnumT, typename ... AspectTs> EnumT
EnumParameter<EnumT, AspectTs...>::str_to_enum( const std::string & str ) {
    _assert_entries_initialized();
    auto it = _entriesPtr->find( str );
    if( _entriesPtr->end() == it ) {
        emraise( notFound, "String \"%s\" does not match to any known "
            "entries of enumeration \"%s\".", str.c_str(), typeid(Enum).name() );
    }
    return it->second;
}


template<typename EnumT, typename ... AspectTs> std::string
EnumParameter<EnumT, AspectTs...>::enum_to_str( EnumT eVal ) {
    _assert_entries_initialized();
    for( auto p : *_entriesPtr ) {
        if( p.second == eVal ) {
            return p.first;
        }
    }
    emraise( notFound, "Couldn't find match for value %d of enum type "
        "\"%s\".", (int) eVal, typeid(Enum).name() );
}

template<typename EnumT, typename ... AspectTs> void
EnumParameter<EnumT, AspectTs...>::add_entry( const std::string & strVal, EnumT eVal ) {
    if( !_entriesPtr ) {
        _entriesPtr = new Entries();
    }
    auto ir = _entriesPtr->emplace( strVal, eVal );
    if( !ir.second ) {
        emraise( nonUniq, "Duplicate insertion of entry \"%s\" for enum type \"%s\".",
            strVal.c_str(), typeid(Enum).name() );
    }
}

template<typename EnumT, typename ... AspectTs> std::vector<std::string>
EnumParameter<EnumT, AspectTs...>::available_values() {
    _assert_entries_initialized();
    std::vector<std::string> ret;
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
# define GOO_ENUM_PARAMETER_DEFINE_XXX( entry, entryName )                      \
static void __goo_define_enum_entry_ ## entryName() __attribute__((__constructor__(155))); \
static void __goo_define_enum_entry_ ## entryName()  {                          \
    ::goo::dict::EnumParameter<decltype(entry)>                                 \
        ::add_entry( STRINGIFY_MACRO_ARG(entryName), entry );                   \
}

# define __GOO_ENUM_EXPAND_ENTRY( entry, scope )                                \
    ::goo::dict::EnumParameter<decltype(scope entry)>                           \
        ::add_entry( STRINGIFY_MACRO_ARG(entry), scope entry );                 \

# define GOO_ENUM_PARAMETER_DEFINE( scope, enumName, substMacro )               \
static void __goo_define_enum_entry_ ## enumName () __attribute__((__constructor__(155))); \
static void __goo_define_enum_entry_ ## enumName ()  {                          \
    substMacro( __GOO_ENUM_EXPAND_ENTRY, scope )                                \
}

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_PARAMETER_ENUM_H


