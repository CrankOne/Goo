# ifndef H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

# include "goo_types.h"
# include "goo_exception.hpp"
# include <list>
# include <cassert>

namespace goo {
namespace dict {

template<typename ValueT>
class iParameter;

/**@class iAbstractParameter
 * @brief An abstract parameter is a base class for most entity in
 * Goo's dictionaries
 *
 * We're following here to POSIX convention, so option-arguments are
 * never optional:
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html#tag_12_02
 *
 * The following subclasses are expected:
 *  - single-char keys (here referred as `shortcuts')   (flags or with values, multiple time)
 *  - long options                                      (flags or with values)
 *  - unnamed positional arguments
 *  - sections (dictionaries itself)
 */
class iAbstractParameter /* {{{ */ {
public:
    typedef UByte ParameterEntryFlag;
    static const ParameterEntryFlag
            set,
            option,
            positional,
            atomic,
            singular,
            required,
            shortened
        ;
private:
    char * _name,           ///< Name of the option. Can be set to nullptr.
         * _description;    ///< Description of the option. Can be set to nullptr.
    /// Stores logical description of an instance.
    ParameterEntryFlag _flags;
protected:
    /// Used only when shortened flag is set.
    char _shortcut;
protected:
    /// Sets the «set» flag translating instance to initialized state.
    void _set_set_flag();

    /// Sets the «required» flag marking a mandatory parameter.
    void _set_required_flag();

    /// This method is to be used by lists only.
    void _unset_singular();

    /// Single ctr can be only invoked by descendants.
    iAbstractParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0');
public:
    virtual ~iAbstractParameter();

    /// Returns pointer to name string.
    const char * name() const;

    /// Returns pointer to description string.
    const char * description() const;

    /// Returns shortcut if it was set. Otherwise returns '\0'.
    char shortcut() const { return _shortcut; }

    /// Returns true, if parameter has a value set (even if it is a default one).
    bool is_set() const {
            return _flags & set;
        }
    /// Returns true, if parameter means logical flags (e.g. is option).
    bool is_option() const {
            return _flags & option;
        }
    /// Returns true, if parameter expects a value (or already contain a default one).
    bool has_value() const {
            return !is_option();
        }
    /// Returns true, if parameter has no name (at all --- even one-letter shortcut).
    bool is_positional() const {
            return _flags & positional;
        }
    /// Returns true, if parameter is not a dictionary.
    bool is_atomic() const {
            return _flags & atomic;
        }
    /// Returns true, if parameter is a dictionary.
    bool is_dictionary() const {
            return is_atomic();
        }
    /// Returns true, if parameter has only one value.
    bool is_singular() const {
            return _flags & singular;
        }
    /// Returns true, if parameter has a set of values.
    bool has_multiple_values() const {
            return !is_singular();
        }
    /// Returns true, if parameter is mandatory.
    bool is_mandatory() const {
            return _flags & required;
        }
    /// Returns true, if parameter can be omitted.
    bool is_optional() const {
            return !is_mandatory();
        }
    /// Returns true, if parameter has a single-character shortcut.
    bool has_shortcut() const {
            return _flags & shortened;
        }
};  /*}}}*/ // class iAbstractParameter




/* @class iSingularParameter
 * @brief Generic command-line parameter features interface.
 *
 * At this level of inheritance the less-generic definition of
 * parameter dictionary entries is defined.
 */
class iSingularParameter : public iAbstractParameter /*{{{*/ {
protected:
    virtual void _V_parse_argument( const char * ) = 0;
    virtual std::string _V_to_string() const = 0;
public:
    /// Returns single-char shortcut for this parameter.
    char shortcut() const { return _shortcut; }

    /// Parses argument from string representation.
    void parse_argument( const char * strval ) {  _V_parse_argument( strval ); }

    /// Forms a human-readable string to be displayed at logs.
    std::string to_string() const { return _V_to_string(); }

    iSingularParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0' );

    /// Getter method.
    template<typename T> const T &
    as() const {
        auto ptr = dynamic_cast<iParameter<T> const *>(this);
        if( !ptr ) {
            if( this->name() ) {
                emraise(badCast, "Couldn't cast parameter \"%s\" to specified type.", name() );
            } else {
                emraise(badCast, "Couldn't cast parameter '%c' to specified type.", shortcut() );
            }
        }
        return ptr->value();
    }
};  // }}} class iSingularParameter




/**@class iParameter
 * @brief A valued intermediate class representing dictionary entry with
 * value of certain type.
 *
 * Defines common value-operation routines.
 * */
template<typename ValueT>
class iParameter : public iSingularParameter /*{{{*/ {
public:
    typedef ValueT Value;
private:
    Value _value;
protected:
    void _set_value( const ValueT & );
    virtual void _V_parse_argument( const char * strval ) override {
        _set_value( _V_parse( strval ) ); }
    virtual std::string _V_to_string( ) const override {
        assert( is_set() );
        return _V_stringify_value( value() ); }

    virtual Value _V_parse( const char * ) const = 0;
    virtual std::string _V_stringify_value( const Value & ) const = 0;
public:
    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ = '\0' );

    iParameter( const char * name,
                const char * description,
                const ValueT & defaultValue,
                ParameterEntryFlag,
                char shortcut_ = '\0' );

    ~iParameter() {}

    const ValueT & value() const;
};  // class iParameter

template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description,
                                ParameterEntryFlag flags,
                                char shortcut_ ) :
        iSingularParameter( name, description, flags, shortcut_ ) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description,
                                const ValueT & defaultValue,
                                ParameterEntryFlag flags,
                                char shortcut_ ) :
        iSingularParameter( name, description, flags, shortcut_ ),
        _value(defaultValue) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT> const ValueT &
iParameter<ValueT>::value() const {
    return _value;
}

template<typename ValueT> void
iParameter<ValueT>::_set_value( const ValueT & val ) {
    _set_set_flag();
    _value = val;
}




/**@class Parameter
 * @brief User-side implementation class. Extension point for user parameters type.
 *
 * There is no default implementation of this class --- only standard C classes
 * are implemented in Goo library.
 */
template<typename ValueT>
class Parameter {
    //std::static_assert<false>( "dft" );
};  // Defailt implementation is empty.




/**@brief
 * 
 * Ctrs are heavily restricted here.
 * TODO: track `default' values overriding.
 */
template<typename ValueT>
class Parameter<std::list<ValueT> > : protected Parameter<ValueT> {
private:
    std::list<ValueT> _values;
protected:
    virtual void _V_push_value( const ValueT & v ) {
        _values.push_back( v ); }

    virtual void _V_parse_argument( const char * strval ) override {
        Parameter<ValueT>::_V_parse_argument( strval );
        _V_push_value( Parameter<ValueT>::value() ); }
public:
    const std::list<ValueT> & values() const { return _values; }

    template<class ... Types>
    Parameter( const std::initializer_list<ValueT> & il, Types ... args ) :
        Parameter<ValueT>( args ... , *il.begin() )
    { /* TODO: push back all the default values */
        this->_unset_singular();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() ); }

    template<class ... Types>
    Parameter( Types ... args ) :
        Parameter<ValueT>( args ... )
    {   this->_unset_singular();
        assert( !(this->name() == nullptr && !this->has_shortcut()) );
        assert( this->description() ); }

    ~Parameter() {}

    friend class InsertionProxy;

    using iAbstractParameter::name;
    using iAbstractParameter::description;
    using iAbstractParameter::shortcut;
    // ... whatever?
};

//
// Specializations
/////////////////

/**@brief Logic option argument type.
 *
 * Most common usage of this object is to provide logical flag for
 * enabling/disabling some functionality. E.g.:
 *      $ do_something -v
 * Or, with equivalent meaning:
 *      $ do_something --verbose=true
 * Besides of true/false values, the following synonims are accepted:
 *      true : True, TRUE, yes, Yes, YES, enable, Enable, ENABLE, on, On, ON
 *      false: False, FALSE, no, No, NO, disable, Disable, DISABLE, off, Off, OFF
 * Note, that this kind of argument can not be required.
 * Possible construction variants:
 *      .p<bool>( 'v', "Enables verbose output" )
 *      .p<bool>( 'q', "Be quiet", true )
 *      .p<bool>( "quet", "Be quiet", true )
 *      .p<bool>( "verbose", "Enables verbose output" )
 *      .p<bool>( 'v', "verbose", "Enables verbose output" )
 *      .p<bool>( 'q', "quet", "Be quiet", true )
 * If there are default argument specified, the argument for such kind of an option
 * is mandatory. Otherwise, it must be omitted. E.g.:
 *   o to enable `.p<bool>( 'v', "Enables verbose output" )` it is enough to:
 *      $ myprogram -v
 *   o contraversary, for .p<bool>( 'q', "Be quiet", true ) it is necessary to:
 *      $ myprogram -q false
 * This aspect affects generated help message.
 * */
template<>
class Parameter<bool> : public iParameter<bool> {
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_ );
    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_ );
    /// Only short option.
    Parameter( char shortcut_,
               const char * description_ );
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               bool default_ );
    /// Long option with shortcut.
    Parameter( char shortcut_,
               const char * name_,
               const char * description_,
               bool default_ );
    /// Only short option.
    Parameter( char shortcut_,
               const char * description_,
               bool default_ );

    // This is to prevent implicit conversion from char * to bool --- fails at compile time.
    Parameter( const char *,
               const char *,
               const char * );
    // This is to prevent implicit conversion from char * to bool --- fails at compile time.
    Parameter( char,
               const char *,
               const char *,
               const char * );

    /// This method is used mostly by Configuration class.
    virtual void set_option( bool );

protected:
    /// Sets parameter value from string. Following strings are acceptable
    /// with appropriate meaning (case insensitive):
    /// (true|enable|on|yes|1)    logically corresponds to `true';
    /// (false|disable|off|no|0)  logically corresponds to `false'.
    virtual Value _V_parse( const char * ) const override;

    /// Returns 'True' or 'False' depending on current value.
    virtual std::string _V_stringify_value( const Value & ) const override;
};

}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

