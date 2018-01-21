//
// Created by crank on 12.01.18.
//

#ifndef GOO_PARAMETERS_SINGULAR_PARAMETER_TCC
#define GOO_PARAMETERS_SINGULAR_PARAMETER_TCC

# include "goo_exception.hpp"
# include "goo_dict/parameter_values.hpp"
# include "goo_dict/app_conf_info.hpp"

namespace goo {
namespace dict {

# if 0
/**@class iSingularParameter
 * @brief Generic parameter features interface.
 *
 * At this level of inheritance the less-generic definition of
 * parameter dictionary entries is defined.
 *
 * @ingroup appParameters
 */
class iSingularParameter : public iBaseValue
                        , virtual public iStringConvertibleParameter {
protected:
    virtual void _V_assign( const iSingularParameter & ) = 0;
public:
    iSingularParameter( const char * name,
                        const char * description,
                        ParameterEntryFlag flags,
                        char shortcut = '\0' );

    virtual void assign( const iSingularParameter & src ) {
        _V_assign( src );
    }

    iSingularParameter & operator=( const iSingularParameter & src ) {
        assign( src );
        return *this;
    }
};  // class iSingularParameter
# endif


//
// Known type-handling abstraction layer
// ////////////////////////////////////

namespace aspects {
/**@brief Default template implementating value parameters that may be converted
 * to string.
 *
 * One may be interested in partial specialization of this class to support
 * various formatting modifiers.
 * */
template<typename ValueT>
class TStringConvertible : public iStringConvertible {
public:
    typedef ValueT Value;
    typedef typename iStringConvertible::ConversionTraits<Value> ValueTraits;
    typedef TStringConvertible<ValueT> Self;
protected:
    /// Sets the value kept by current instance from string expression.
    virtual void _V_parse_argument(const char *strval) override {
        static_cast<TValue<ValueT>*>(this)->value(ValueTraits::parse_string_expression(strval));
    }

    /// Expresses the value kept by current instance as a string.
    virtual std::string _V_to_string() const override {
        return ValueTraits::to_string_expression(*static_cast<TValue<ValueT>*>(this));
    }
public:
    TStringConvertible() {}
    explicit TStringConvertible(const ValueT &v) : TValue<Value>(v) {}
    explicit TStringConvertible(const Self &o) : TValue<Value>(o) {}
};

}  // namespace aspects

# if 0
/**@class iParameter
 * @brief A valued intermediate class representing dictionary entry with
 *        value of certain type.
 *
 * Junction class of the iTStringConvertibleParameter template with
 * iSingularParameter defining annotated parameter structure used by
 * parameter dictionaries (annotated hashing containers).
 * */
template<typename ValueT>
class iParameter : public iSingularParameter
                 , public iTStringConvertibleParameter<ValueT> {
public:
    typedef iTStringConvertibleParameter<ValueT> ValueStoringType;
    typedef typename ValueStoringType::Value Value;
    typedef iSingularParameter::ParameterEntryFlag ParameterEntryFlag;
protected:
    virtual void _set_value( const ValueT & ) override;
    virtual void _V_assign( const iSingularParameter & ) override;
public:
    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ = '\0' );

    iParameter( const char * name,
                const char * description,
                ParameterEntryFlag,
                char shortcut_ /*= '\0' allowed */,
                const ValueT & defaultValue );

    iParameter( const iParameter<ValueT> & );

    ~iParameter() {}

    /// Const value getter (public). Raises "unitialized" error if value was
    /// not set.
    virtual const ValueT & value() const;
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
                                ParameterEntryFlag flags,
                                char shortcut_,
                                const ValueT & defaultValue ) :
        iSingularParameter( name, description, flags, shortcut_ ),
        ValueStoringType(defaultValue) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT>
iParameter<ValueT>::iParameter( const iParameter<ValueT> & o ) :
        iSingularParameter(o),
        ValueStoringType(o) {
    // Checks for consistency:
    /* ... */
}

template<typename ValueT> const ValueT &
iParameter<ValueT>::value() const {
    if( !is_set() ) {
        char vlOnShortcut[2] = {'\0', '\0'};
        if( !name() && has_shortcut() ) {
            vlOnShortcut[0] = shortcut();
        }
        throw goo::TheException<goo::Exception::uninitialized>(
                name() ? name() : vlOnShortcut,
                &(value()), "Parameter value is not set." );
    }
    return ValueStoringType::value();
}

template<typename ValueT> void
iParameter<ValueT>::_V_assign( const iSingularParameter & spVal ) {
    if( !spVal.is_set() ) {
        // The original value is not set. The expected behaviour in this case
        // is to do nothing with own value. TODO: this has to be reflected in
        // documentation!
        return;
    }
    iSingularParameter::_set_is_set_flag();

    // TODO: dynamic_cast<> here may ruin the performance for complex types.
    auto p = dynamic_cast<const iParameter<ValueT> *>( &spVal );
    if( !p ) {
        emraise( badCast, "Types mismatch. Unable to perform assignment of "
            "parameter instance \"%s\" (with value =%s) to iParameter<%s> "
            "instance.", spVal.target_type_info().name(),
            spVal.to_string().c_str(), typeid(ValueT).name() );
    }
    _set_value( p->value() );
}

template<typename ValueT> void
iParameter<ValueT>::_set_value( const ValueT & val ) {
    iSingularParameter::_set_is_set_flag();
    iTValue<ValueT>::_set_value(val);
}
# endif

}  // namespace dict
}  // namespace goo

# endif  // GOO_PARAMETERS_SINGULAR_PARAMETER_TCC
