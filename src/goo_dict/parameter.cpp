# include "goo_dict/parameter.tcc"
# include "goo_exception.hpp"

# include <cstring>

namespace goo {
namespace dict {

const iAbstractParameter::ParameterEntryFlag
    iAbstractParameter::set         = 0x1,      // otherwise --- uninitialized still
    iAbstractParameter::flag        = 0x2,      // otherwise --- requires an argument
    iAbstractParameter::positional  = 0x4,      // otherwise --- has name or shortcut
    iAbstractParameter::atomic      = 0x8,      // otherwise --- it is a dictionary
    iAbstractParameter::singular    = 0x10,     // otherwise --- can be repeated multiple times
    iAbstractParameter::required    = 0x20,     // otherwise --- is optional
    iAbstractParameter::shortened   = 0x40;     // otherwise --- has not one-char shortcut

iAbstractParameter::iAbstractParameter( const char * name_,
                                        const char * description_,
                                        ParameterEntryFlag flags,
                                        char shortcut_ ) :
                                        _name(nullptr),
                                        _flags( flags ),
                                        _shortcut( shortcut_ ) {
    const size_t nLen = name_ ? strlen(name_) : 0;
    // Checks for consistency:
    if( !has_shortcut() && '\0' != _shortcut ) {
        emraise( badState, "Wrong parameter flags set by child ctr: "
            "shortcut is provided but flag is not set.");
    }
    if( '\0' != shortcut_ && !isalnum(shortcut_) ) {
        emraise( malformedArguments,
                 "Won't create entry with shortcut of code %d (non-alnum).",
                 (int) shortcut_ );
    }
    if( name_ && !nLen ) {
        emraise( malformedArguments,
                 "Won't create entry without name (if you want to omit long name, use NULL)." );
    }
    if( name_ ) {
        unsigned char n = 0;
        for( const char * c = name_; '\0' != *c; ++c, ++n ) {
            if( !isdigit(*c) ) break;
            if( n > 1 ) {
                // See Guideline 3 of POSIX convention; 12.2 "Utility Syntax Guidelines".
                emraise( malformedArguments,
                     "Multi-digit options are prohibited." );
            }
        }
    }
    //if( !description_ ) {
    //    emraise( malformedArguments,
    //             "Won't create entry with NULL description." );
    //}
    /* ... */
    if( name_ ) {
        name( name_ );
    } else {
        _name = nullptr;
    }

    if( description_ ) {
        const size_t dLen = strlen(description_);
        _description = new char [dLen + 1];
        strncpy( _description, description_, dLen + 1 );
    } else {
        _description = nullptr;
    }
}

iAbstractParameter::iAbstractParameter( const iAbstractParameter & o ) {
    //memcpy( this, &o, sizeof(o) );  // TODO: find a better solution b'cause overwriting
    //                                // zee vtable can be dangerous!
    this->_flags = o._flags;
    this->_shortcut = o._shortcut;
    const size_t nLen = (o._name ? strlen( o._name ) + 1 : 0),
                 dLen = (o._description ? strlen( o._description ) + 1 : 0)
                 ;
    if(nLen) {
        _name = new char [nLen];
        memcpy( _name, o._name, nLen );
    } else {
        _name = nullptr;
    }

    if( o._description ) {
        assert( dLen );
        _description = new char [dLen];
        memcpy( _description, o._description, dLen );
    } else {
        _description = nullptr;
    }
}

iAbstractParameter::~iAbstractParameter() {
    if( _name ) {
        delete [] _name;
    }
    if( _description ) {
        delete [] _description;
    }
}

void
iAbstractParameter::name( const char * name_ ) {
    size_t nLen = strlen( name_ ) + 1;
    if( _name ) {
        delete [] _name;
    }
    _name = new char [nLen + 1];
    strncpy( _name, name_, nLen + 1 );
}

void
iAbstractParameter::_check_initial_validity() {
    if( '\0' != _shortcut && 'W' == _shortcut ) {
        // See Guideline 3 of POSIX convention; 12.2 "Utility Syntax Guidelines".
        emraise( malformedArguments,
                 "The -W (capital-W) option is reserved for vendor options." );
    }
    if( '\0' == _shortcut && !_name && is_atomic() ) {
        emraise( malformedArguments,
                 "Won't create entry without long name and shortcut." );
    }
    if( has_shortcut() && is_positional() ) {
        emraise( malformedArguments,
                 "Wrong entry description: positional and has shortcut." );
    }
    if( is_set() && is_mandatory() ) {
        emraise( malformedArguments,
                 "Wrong entry description: is mandatory and has default value." );
    }
    // =
    // positional && !atomic    ...
    // positional && shortened  shortcuts for positional argument is nonsense
    // positional && name()     name for positional argument is nonsense
    if( is_positional() && (_name || has_shortcut()) ) {
        emraise( badState,  // design error
                 "Name or shortcut is provided for positional argument." );
    }
    // other prohibited cases:
    // =
    // flag && !set             flag are always set to false by default
    // flag && required         flag can not be required
    // flag && positional       flag never can be positional argument (satisfied above)
    // flag && !atomic          flag is always a two-state object
    if( is_flag() && (!is_set()
                    || is_mandatory()
                    || is_dictionary() ) ) {
        emraise( malformedArguments,
                 "Tristate flags are not supported (is flag, "
                 "is set=%c, "
                 "is mandatory=%c, "
                 "is dictionary=%c, "
                 ").",
                 is_set() ? 'y' : 'n',
                 is_mandatory() ? 'y' : 'n',
                 is_dictionary() ? 'y' : 'n' );
    }
    // =
    // set && required          (only on initial state) can not require having a default value
    if( is_set() && is_mandatory() ) {
        emraise( malformedArguments,
                 "Default value can not be set for required parameter.");
    }
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
iAbstractParameter::_append_description( const char * d ) {
    assert( d );
    size_t newDLength = strlen( d );
    if( _description ) {
        char * newd = new char [newDLength + strlen(_description) + 1];
        strcpy( newd, _description );
        strcpy( newd + strlen(_description), d );
        _description = newd;
    } else {
        _description = new char [newDLength + 1];
        strcpy( _description, d );
    }
}

void
iAbstractParameter::_set_is_set_flag() {
    _flags |= set;
}

void
iAbstractParameter::_set_is_flag_flag() {
    _flags |= flag;
}

void
iAbstractParameter::_unset_singular() {
    _flags &= ~singular;
}

void
iAbstractParameter::set_is_argument_required_flag() {
    _flags |= required;
}

//
// Singular parameter
////////////////////

iSingularParameter::iSingularParameter( const char * name_,
                                        const char * description_,
                                        ParameterEntryFlag flags_,
                                        char shortcut_ ) :
        iAbstractParameter( name_, description_, flags_, shortcut_ ) {}

}  // namespace dict
}  // namespace goo

