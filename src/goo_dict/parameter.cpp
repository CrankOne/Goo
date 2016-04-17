# include "goo_dict/parameter.tcc"
# include "goo_exception.hpp"

# include <cstring>
# include <regex>

namespace goo {
namespace dict {

static const std::regex
        _static_logicalTermRegex_True(  "(true|enable|on|yes|1)",   std::regex_constants::icase ),
        _static_logicalTermRegex_False( "(false|disable|off|no|0)", std::regex_constants::icase );

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
                 "Won't create entry without long name and shortcut." );
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

void
iAbstractParameter::_set_set_flag() {
    _flags |= set;
}

void
iAbstractParameter::_unset_singular() {
    _flags &= ~singular;
}

//
// Singular parameter
////////////////////

iSingularParameter::iSingularParameter( const char * name_,
                                        const char * description_,
                                        ParameterEntryFlag flags_,
                                        char shortcut_ ) :
        iAbstractParameter( name_, description_, flags_, shortcut_ ) {}

//
// Logic values (option)
///////////////////////

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
                              default_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}

Parameter<bool>::Parameter( char shortcut_,
                            const char * description_,
                            bool default_ ) :
            iParameter<bool>( nullptr,
                              description_,
                              default_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}

void
Parameter<bool>::set_option( bool a ) {
    _set_value( true );
}

bool
Parameter<bool>::_V_parse( const char * argStr ) const {
    // _static_logicalTermRegex_True
    if( std::regex_match( argStr, _static_logicalTermRegex_True ) ) {
        return true;
    } else if( std::regex_match( argStr, _static_logicalTermRegex_False) ) {
        return false;
    } else {
        emraise( badParameter,
                 "Expression \"%s\" can not be parsed as logical option.",
                 argStr );
    }
}

std::string
Parameter<bool>::_V_stringify_value( const bool & val ) const {
    if( val ) {
        return "True";
    } else {
        return "False";
    }
}

}  // namespace dict
}  // namespace goo

