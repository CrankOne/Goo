//
// Created by crank on 12.01.18.
//

#ifndef GOO_PARAMETERS_VALUES_HPP
#define GOO_PARAMETERS_VALUES_HPP

# include "goo_dict/types.hpp"
# include "goo_mixins/vcopy.tcc"

namespace goo {
namespace dict {

/**@brief Type-agnostic value parameter base.
 *
 * This abstract base claims interface of type-agnostic value-bearing parameter
 * instance providing generic methods accessing the data memory address and
 * C++ RTTI information.
 * */
template<typename ... AspectTs>
class iBaseValue : public mixins::iDuplicable< iBaseValue<AspectTs...>
                                            , iBaseValue<AspectTs...>
                                            , iBaseValue<AspectTs...>
                                            > {
public:
    typedef iBaseValue<AspectTs ...> Self;
private:
    // ...
protected:
    /// Shall return untyped data pointer.
    virtual void * _V_data_addr() = 0;
    /// Shall return untyped (const) data pointer.
    virtual const void * _V_data_addr() const = 0;
    /// Shall return C++ RTTI type info.
    virtual const std::type_info & _V_target_type_info() const = 0;
public:
    iBaseValue() = default;
    iBaseValue( const Self & o ) = default;
    /// Returns untyped data pointer.
    void * data_addr() { return _V_data_addr(); }
    /// Returns untyped (const) data pointer.
    const void * data_addr() const { return _V_data_addr(); }
    /// Returns C++ RTTI value type info.
    virtual const std::type_info & target_type_info() const
                { return _V_target_type_info(); }

    template<typename T> T & aspect_cast() {
        return dynamic_cast<T&>(*this);  // TODO: cache cast
    }

    // note: for this two methods see implementation at the end of the header.
    // They use downcast operations with types that are incomplete at the
    // moment.
    /// Getter method (convinience, implemented in parameter.tcc).
    template<typename T> const T & as() const;
    /// Getter method (convinience, implemented in parameter.tcc) for array
    /// parameters.
    template<typename T> const Array<T> & as_array_of() const;
    // TODO: ^^^ rename to as_array_of
};  // class iBaseValue


/**@brief Template container for a value.
 * @class iTValue
 *
 * Represents a value-keeping aspect of the parameter classes.
 * */
template<typename ValueT, typename ... AspectTs>
class TValue : public virtual iBaseValue<AspectTs...> {
    friend class LoDInsertionProxy;
public:
    typedef ValueT Value;
private:
    Value _value;
protected:
    /// Mutable value reference getter.
    ValueT & _mutable_value() { return _value; }
    virtual const std::type_info & _V_target_type_info() const final {
        return typeid(ValueT); }
    /// This method is to be used by user code, for special cases. It should
    /// not be used by Goo API, during the normal argument parsing cycle.
    virtual void _set_value( const ValueT & v ) { _value = v; }
    /// Returns kept value address.
    virtual void * _V_data_addr() override { return &_value; }
    /// Returns kept value address (const).
    virtual const void * _V_data_addr() const override { return &_value; }
    /// Potentially dangerous ctr leaving the value uninitialized.
    TValue() : _value() {}
public:
    /// Const value getter (public).
    virtual const ValueT & value() const { return _value; }
    virtual void value(const Value & v) { _set_value(v); }
    explicit TValue( const ValueT & v ) : _value(v) {}
    TValue( const TValue<Value> & o ) : _value(o._value) {}
};

}  // namespace dict
}  // namespace goo

#endif //GOO_VALUES_HPP
