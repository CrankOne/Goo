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


}  // namespace dict
}  // namespace goo

