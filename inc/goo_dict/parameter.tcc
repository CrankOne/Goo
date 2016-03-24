# ifndef H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H
# define H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

# include "goo_types.h"

namespace goo {
namespace dict {

class iAbstractParameter {
private:
    char * _name,
         * _description;
    bool _isSet;
    bool _hasDefault;
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
                        bool hasDefault );
    virtual ~iAbstractParameter();
public:
    const char * name() const;
    const char * description() const;
    bool has_default() const;

    void from_string( const char * );
    void deserialize( const UByte * );
    Size serialized_length() const;
    void serialize( UByte * ) const;
    size_t string_length() const;
    void to_string( char * ) const;

    virtual bool is_section() const { return false; }
};  // class iAbstractParameter


template<typename ValueT>
class iParameter : public iAbstractParameter {
public:
    typedef ValueT Value;
private:
    Value _value;
protected:
    void _set_value( const ValueT & );
public:
    iParameter( const char * name,
                const char * description);
    iParameter( const char * name,
                const char * description,
                const ValueT & defaultValue);
    ~iParameter() {}

    const ValueT & value() const;
};  // class iParameter


template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description ) :
        iAbstractParameter(name, description, false) {
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const char * name,
                                const char * description,
                                const ValueT & defaultValue ) :
        iAbstractParameter(name, description, true),
        _value(defaultValue) {
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
};
# endif


}  // namespace dict
}  // namespace goo

# endif  // H_GOO_PARAMETERS_DICTIONARY_DICTIONARY_H

