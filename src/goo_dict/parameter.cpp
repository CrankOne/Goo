# include "goo_dict/parameter.tcc"
# include "goo_exception.hpp"

# include <cstring>

namespace goo {
namespace dict {

const iAbstractParameter::ParameterEntryFlag
    iAbstractParameter::set         = 0x1,      // otherwise --- uninitialized still
    iAbstractParameter::option      = 0x2,      // otherwise --- has value
    iAbstractParameter::positional  = 0x4,      // otherwise --- has name or shortcut
    iAbstractParameter::atomic      = 0x8,      // otherwise --- it is a dictionary
    iAbstractParameter::singular    = 0x10,     // otherwise --- can be repeated multiple times
    iAbstractParameter::required    = 0x20,     // otherwise --- is optional
    iAbstractParameter::shortened   = 0x40;     // otherwise --- has not one-char shortcut

// XXX
//const iAbstractParameter::ParameterEntryFlag
//    iAbstractParameter::option_baseFlags = 0x0 |
//        iAbstractParameter::atomic

iAbstractParameter::iAbstractParameter( const char * name_,
                                        const char * description_,
                                        ParameterEntryFlag flags,
                                        char shortcut_ ) :
                                        _flags( flags ),
                                        _shortcut( shortcut_ ) {
    const size_t nLen = name_ ? strlen(name_) : 0;
    // Checks for consistency:
    if( '\0' != shortcut_ && !isalnum(shortcut_) ) {
        emraise( malformedArguments,
                 "Won't create entry with shortcut of code %d.", (int) shortcut_ );
    }
    if( '\0' == shortcut_ && !name_ && is_atomic() ) {
        emraise( malformedArguments,
                 "Won't create entry \"%s\" without name_ and shortcut.", name_ );
    }
    if( name_ && !nLen ) {
        emraise( malformedArguments,
                 "Won't create entry without name (if you want to omit long name, use NULL)." );
    }
    if( !description_ ) {
        emraise( malformedArguments,
                 "Won't create entry with NULL description." );
    }
    if( has_shortcut() && is_positional() ) {
        emraise( malformedArguments,
                 "Wrong entry description: positional and has shortcut." );
    }
    if( is_set() && is_mandatory() ) {
        emraise( malformedArguments,
                 "Wrong entry description: is mandatory and has default value." );
    }
    /* ... */
    const size_t dLen = strlen(description_);
    if( name_ ) {
        _name = new char [nLen + 1];
        strncpy( _name, name_, nLen + 1 );
    } else {
        _name = nullptr;
    }
    _description = new char [dLen + 1];
    strncpy( _description, description_, dLen + 1 );
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

# if 0  // XXX
# define require_uninitialized \
if(!is_set()) { emraise( badState, "Pre-initialization operation invoked for initialized parameter instance." ); }
# define require_initialized \
if(is_set()) { emraise( badState, "Post-initialization operation invoked for uninitialized parameter instance." ); }

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

# endif

# if 0
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
# endif

# if 0
Parameter<bool>::Parameter( char shortcut,
                            const char * name,
                            const char * description ) :
            iParameter<bool>( name,
                              description,
                              iAbstractParameter::atomic |
                              ( !name ? iAbstractParameter::positional : 0x0),
                              shortcut ) {}
# endif

Parameter<bool>::Parameter( const char * name_,
                            const char * description_ ) :
            iParameter<bool>( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::option
                            ) {}


Parameter<bool>::Parameter( char shortcut_,
                            const char * name_,
                            const char * description_ ) :
            iParameter<bool>( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::option |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}

Parameter<bool>::Parameter( char shortcut_,
                            const char * description_ ) :
            iParameter<bool>( nullptr,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::option |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}


Parameter<bool>::Parameter( const char * name_,
                            const char * description_,
                            bool default_ )  :
            iParameter<bool>( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular
                            ) {}


Parameter<bool>::Parameter( char shortcut_,
                            const char * name_,
                            const char * description_,
                            bool default_ ) :
            iParameter<bool>( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              shortcut_,
                              default_
                            ) {}

Parameter<bool>::Parameter( char shortcut,
                            const char * description_,
                            bool default_ ) :
            iParameter<bool>( nullptr,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              '\0',
                              default_
                            ) {}

}  // namespace dict
}  // namespace goo

