# include "goo_dict/parameter.tcc"
# include "goo_exception.hpp"

# include <cstring>

namespace goo {
namespace dict {

iAbstractParameter::iAbstractParameter( const char * name,
                                        const char * description,
                                        bool hasDefault ) :
                        _isSet(false), _hasDefault( hasDefault ) {
    const size_t nLen = strlen(name),
                 dLen = strlen(description);
    name = new char [nLen + 1];
    description = new char [dLen + 1];
    strncpy( _name, name, nLen );
    strncpy( _description, description, dLen );

}

iAbstractParameter::~iAbstractParameter() {
    delete [] _name;
    delete [] _description;
}

const char *
iAbstractParameter::name() const {
    return _name;
}

const char *
iAbstractParameter::description() const {
    return _description;
}

bool
iAbstractParameter::has_default() const {
    return _hasDefault;
}

# define require_uninitialized \
if(!_isSet) { emraise( badState, "Pre-initialization operation invoked for initialized parameter instance." ); }
# define require_initialized \
if(_isSet) { emraise( badState, "Post-initialization operation invoked for uninitialized parameter instance." ); }

void
iAbstractParameter::from_string( const char * str ) {
    require_uninitialized;
    _V_from_string( str );
}

void
iAbstractParameter::deserialize( const UByte * bytes ) {
    require_uninitialized;
    _V_deserialize( bytes );
}

Size
iAbstractParameter::serialized_length() const {
    require_initialized;
    return _V_serialized_length();
}

void
iAbstractParameter::serialize( UByte * bytes ) const {
    require_initialized;
    _V_serialize( bytes );
}

size_t
iAbstractParameter::string_length() const {
    require_initialized;
    return _V_string_length();
}

void
iAbstractParameter::to_string( char * str ) const {
    require_initialized;
    _V_to_string( str );
}

# undef require_uninitialized
# undef require_initialized

/*
 * TODO: all arithmetic parameter implementations based
 * on GDS parsing routines.
 */

void
Parameter<bool>::_V_from_string( const char * str ) {
    if( !strcmp( str, "true" ) ) {
        _set_value(true);
    } else if( !strcmp( str, "false" ) ) {
        _set_value(false);
    } else {
        emraise( badParameter,
            "%s seems to be not a logic-type argument.",
            str );
    }
}

void
Parameter<bool>::_V_deserialize( const UByte * series ) {
    if( *series == 0x0 ) {
        _set_value( false );
    } else if( *series == 0x1 ) {
        _set_value( true );
    } else {
        emraise( corruption,
            "Got %u numeric value in byte while logic value expected.",
            (unsigned int) *series );
    }
}

Size
Parameter<bool>::_V_serialized_length() const {
    return 1;
}

void
Parameter<bool>::_V_serialize( UByte * series ) const {
    if( value() ) {
        series[0] = 0x1;
    } else {
        series[0] = 0x0;
    }
}

size_t
Parameter<bool>::_V_string_length() const {
    if( value() ) {
        return 4;  // for `true'
    } else {
        return 5;  // for `false'
    }
}

void
Parameter<bool>::_V_to_string( char * str ) const {
    if( value() ) {
        strncpy( str, "true", 4 );
    } else {
        strncpy( str, "true", 5 );
    }
}

}  // namespace dict
}  // namespace goo

