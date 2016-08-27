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
    iAbstractParameter::argOpt      = 0x2,      // otherwise --- value can not be omitted (?)
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

iAbstractParameter::iAbstractParameter( const iAbstractParameter & o ) {
    memcpy( this, &o, sizeof(o) );
    const size_t nLen = o._name ? strlen( o._name ) + 1 : 0,
                 dLen = strlen( o._description ) + 1
                 ;
    _name = new char [nLen];
    if( nLen ) { memcpy( _name, o._name, nLen ); }

    _description = new char [dLen];
    memcpy( _description, o._description, dLen );
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
iAbstractParameter::_set_required_flag() {
    _flags |= required;
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
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::argOpt
                            ) {}


Parameter<bool>::Parameter( char shortcut_,
                            const char * name_,
                            const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::argOpt |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}

Parameter<bool>::Parameter( char shortcut_,
                            const char * description_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::argOpt |
                              iAbstractParameter::shortened,
                              shortcut_
                            ) {}


Parameter<bool>::Parameter( const char * name_,
                            const char * description_,
                            bool default_ )  :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular,
                              '\0',
                              default_
                            ) {}


Parameter<bool>::Parameter( char shortcut_,
                            const char * name_,
                            const char * description_,
                            bool default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              shortcut_,
                              default_
                            ) {}

Parameter<bool>::Parameter( char shortcut_,
                            const char * description_,
                            bool default_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::set |
                              iAbstractParameter::atomic |
                              iAbstractParameter::singular |
                              iAbstractParameter::shortened,
                              shortcut_,
                              default_
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
                 "Could not interpret \"%s\" as a logical option.",
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


//
// Integer values (option)
/////////////////////////

Parameter<int>::Parameter( const char * name_,
                           const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                            ) {}

Parameter<int>::Parameter( char shortcut_,
                      const char * name_,
                      const char * description_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::shortened,
                              shortcut_
                            ) {}

Parameter<int>::Parameter( char shortcut_,
                           const char * description_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular,
                              shortcut_
                            ) {}

Parameter<int>::Parameter( const char * name_,
                           const char * description_,
                           int default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              '\0',
                              default_
                            ) {
}

Parameter<int>::Parameter( char shortcut_,
                           const char * name_,
                           const char * description_,
                           int default_ ) :
            DuplicableParent( name_,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              shortcut_,
                              default_
                            ) {}

Parameter<int>::Parameter( char shortcut_,
                           const char * description_,
                           int default_ ) :
            DuplicableParent( nullptr,
                              description_,
                              iAbstractParameter::atomic
                                | iAbstractParameter::singular
                                | iAbstractParameter::set,
                              shortcut_,
                              default_
                            ) {}

int
Parameter<int>::_V_parse( const char * ) const {
    _TODO_ // TODO
}

std::string
Parameter<int>::_V_stringify_value( const Value & ) const {
    _TODO_ // TODO
}

}  // namespace dict
}  // namespace goo

