# ifndef H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

# include "goo_types.h"

namespace goo {
namespace dict {

/**@class iAbstractParameter
 * @brief An abstract parameter is a base class for most entity in
 * Goo's dictionaries
 *
 * The following subclasses are expected:
 *  - single-char keys                  (flags or with values, multiple time)
 *  - long options                      (flags or with values)
 *  - unnamed positional arguments      ()
 *  - sections (dictionaries itself)
 */
class iAbstractParameter {
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
    char * _name,
         * _description;
    ParameterEntryFlag _flags;
protected:
    void set_set_flag();

    virtual void _V_from_string( const char * ) = 0;
    virtual void _V_deserialize( const UByte * ) = 0;


    virtual Size _V_serialized_length() const = 0;
    virtual void _V_serialize( UByte * ) const = 0;

    virtual size_t _V_string_length() const = 0;
    virtual void _V_to_string( char * ) const = 0;

    iAbstractParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags );
    virtual ~iAbstractParameter();
public:
    /// Returns pointer to name string.
    const char * name() const;

    /// Returns pointer to description string.
    const char * description() const;

    /// Sets value of parameter according to ASCII-text expression given.
    void from_string( const char * );

    /// Sets value of parameter according to byte sequence provided.
    void deserialize( const UByte * );

    /// Writes byte sequence representing value.
    void serialize( UByte * ) const;

    /// Returns length of byte sequence representing serialized value.
    Size serialized_length() const;

    /// Returns length of ASCII-text representation.
    size_t string_length() const;

    /// Writes ASCII-text representation by pointer provided.
    void to_string( char * ) const;

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
    bool has_shortened_character() const {
            return _flags & shortened;
        }
};  // class iAbstractParameter

template<typename ValueT>
class iParameter : public iAbstractParameter {
public:
    typedef ValueT Value;
private:
    /// Used only when shortened flag is set.
    char _shortcut;
    Value _value;
protected:
    void _set_value( const ValueT & );
public:
    iParameter( ParameterEntryFlag,
                const char * name,
                char shortcut,
                const char * description );

    iParameter( ParameterEntryFlag,
                const char * name,
                char shortcut,
                const char * description,
                const ValueT & defaultValue );
    ~iParameter() {}

    const ValueT & value() const;
};  // class iParameter


template<typename ValueT>
iParameter<ValueT>::iParameter( ParameterEntryFlag flags,
                                const char * name,
                                char shortcut,
                                const char * description ) :
        iAbstractParameter( name, description, flags ),
        _shortcut(shortcut) {
    // TODO: check for consistency
}

template<typename ValueT>
iParameter<ValueT>::iParameter( ParameterEntryFlag flags,
                                const char * name,
                                char shortcut,
                                const char * description,
                                const ValueT & defaultValue ) :
        iAbstractParameter( name, description, flags ),
        _shortcut(shortcut),
        _value(defaultValue) {
    // TODO: check for consistency, i.e. `required' flag can not be
    // set for default-valued parameter and parameter with dictionary
    // and so on...
}

template<typename ValueT> const ValueT &
iParameter<ValueT>::value() const {
    return _value;
}

template<typename ValueT> void
iParameter<ValueT>::_set_value( const ValueT & val ) {
    _value = val;
}

template<typename ValueT>
class Parameter {};  // No defailt implementation.

// Specializations:

# if 0  // TODO
# define declare_explicit_specialization( typecode, cType, gnm, gdsnm ) \
template<>                                                       \
class Parameter<cType> : public iParameter<cType> {              \
protected:                                                       \
    virtual void _V_from_string( const char * )  override;       \
    virtual void _V_deserialize( const UByte * ) override;       \
    virtual Size _V_serialized_length() const    override;       \
    virtual void _V_serialize( UByte * ) const   override;       \
    virtual size_t _V_string_length() const      override;       \
    virtual void _V_to_string( char * ) const    override;       \
};

for_all_atomic_datatypes( declare_explicit_specialization )

# undef declare_explicit_specialization
# else
template<>
class Parameter<bool> : public iParameter<bool> {
protected:
    virtual void _V_from_string( const char * )  override;
    virtual void _V_deserialize( const UByte * ) override;
    virtual Size _V_serialized_length() const    override;
    virtual void _V_serialize( UByte * ) const   override;
    virtual size_t _V_string_length() const      override;
    virtual void _V_to_string( char * ) const    override;
public:
    Parameter( const char * name,
               const char * description );
    Parameter( const char * name,
               const char * description,
               bool defaultValue );
};
# endif


}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

