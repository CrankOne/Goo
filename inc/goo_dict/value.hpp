/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// Created by crank on 12.01.18.

# ifndef H_GOO_PARAMETERS_VALUES_H
# define H_GOO_PARAMETERS_VALUES_H

# include <tuple>

# include "goo_dict/types.hpp"
# include "goo_mixins/vcopy.tcc"
# include "goo_utility.hpp"

# include <cassert>

namespace goo {
namespace dict {

/**@brief Abstract base class for every parameter or dictionary.
 * @class iAbstractValue
 *
 * This base expresses the most basic idea behind every entity within the
 * goo::dict framework by declaring a class that refers to physical data block.
 * No assumptions about the particular data structure or type are made here.
 *
 * Albeit, this class is rarely used within the framework, it may be convenient
 * or necessary for some rare cases (like virtual copy constructions). See
 * iBaseValue subclass playing more significant role.
 *
 * Unfortunately, C++ has no native means for Aspect-Oriented Programming, nor
 * for the virtual constructor, so we had to implement it idiomatically.
 * */
class iAbstractValue : public mixins::iDuplicable<iAbstractValue, iAbstractValue, iAbstractValue> {
protected:
    /// Shall return untyped data pointer.
    virtual void * _V_data_addr() = 0;
    /// Shall return untyped (const) data pointer.
    virtual const void * _V_data_addr() const = 0;
    /// Shall return C++ RTTI type info.
    virtual const std::type_info & _V_target_type_info() const = 0;
public:
    iAbstractValue() = default;
    iAbstractValue(const iAbstractValue &) = default;
    /// Returns untyped data pointer.
    void * data_addr() { return _V_data_addr(); }
    /// Returns untyped (const) data pointer.
    const void * data_addr() const { return _V_data_addr(); }
    /// Returns C++ RTTI value type info.
    virtual const std::type_info & target_type_info() const
                { return _V_target_type_info(); }
};

namespace aspects {

/**@brief Basic mixin-pointcut for reverse dereferencing.
 * @class BoundMixin
 *
 * This mixin holds the pointer to the owning instance, granting subclassing
 * aspect access to it.
 */
class BoundMixin {
private:
    iAbstractValue * _self;
protected:
    BoundMixin() : _self(nullptr) {}
public:
    void set_target( iAbstractValue * t ) { assert(!_self); _self = t; }
    bool is_target_instance_set() const { return !!_self; }
    iAbstractValue & target() { assert(is_target_instance_set()); return *_self; }
    const iAbstractValue & target() const { assert(is_target_instance_set()); return *_self; }
};

}  // namespace aspects

// aux::unwind_set_targets<ValueT, AspectTs...>( this );

/**@brief Type-agnostic value parameter base.
 * @class iBaseValue
 *
 * This abstract base claims interface of type-agnostic value-bearing parameter
 * instance providing generic methods accessing the data memory address and
 * C++ RTTI information.
 *
 * The particular set of aspects parameters within the template parameter list
 * will define the behaviour of the object at few stages of its lifetime.
 *
 * One may think about this class as it is a pointcut with following join
 * points:
 *  - Setting avalue
 *  - Retrieving a value
 *  - ... todo?
 * */
template<typename ... AspectTs>
class iBaseValue : public iAbstractValue {
public:
    typedef iBaseValue<AspectTs ...> Self;
private:
    std::tuple<AspectTs * ...> _aspects;
public:
    explicit iBaseValue( AspectTs * ... aspects );
    explicit iBaseValue( std::tuple<AspectTs * ...> t );
    //iBaseValue() = default;
    //iBaseValue( const Self & o ) = default; // TODO: copy aspect values instead of just their ptrs

    /// Exposes aspects tuple.
    std::tuple<AspectTs * ...> aspects() { return _aspects; }

    template<typename T> T * aspect_cast() {
        return std::get<stdE::get_type_index<T, AspectTs...>::value>(_aspects);
    }

    template<typename T> const T * aspect_cast() const {
        return std::get<stdE::get_type_index<T, AspectTs...>::value>(_aspects);
    }

    // note: for this two methods see implementation at the end of the header.
    // They use downcast operations with types that are incomplete at the
    // moment.
    /// Getter method (convinience, implemented in parameter.tcc).
    template<typename T>
    typename std::enable_if<stdE::is_one_of<T, AspectTs...>::value, const T &>::type
    as() const { return *aspect_cast<T>(); };

    template<typename T>
    typename std::enable_if< ! stdE::is_one_of<T, AspectTs...>::value, const T &>::type
    as() const;

    /// Getter method (convinience, implemented in parameter.tcc) for array
    /// parameters.
    template<typename T> const Array<T> & as_array_of() const;
    // TODO: ^^^ rename to as_array_of
};  // class iBaseValue

namespace aspects {
namespace aux {

template<typename HeadT>
struct _Unwind {
    struct Setter {
        static int set(HeadT *i, iAbstractValue * t) { i->set_target(t); return 1; }
    };

    struct Dummy {
        static int set(HeadT *, iAbstractValue *) { /* shall be trimmed off by optimization */ return 0; }
    };

    typedef typename std::conditional< std::is_base_of<BoundMixin, HeadT>::value
            , Setter
            , Dummy>::type Doer;
};

// Recursive template aspects unwinding template.
template< int N
        , typename HeadT
        , typename ... TailTs>
struct _TTUnwind {
    template<typename ... AspectTs>
    static void set_tuple_aspect_targets(iAbstractValue * trg, std::tuple<AspectTs * ...> t) {
        _Unwind<HeadT>::Doer::set( std::get<N>(t), trg );
        _TTUnwind<N+1, TailTs...>:: \
                template set_tuple_aspect_targets<AspectTs...>( trg, t );
        //auto rs[] = {  };
    }
};

// Last aspect unwinding (template arguments expansion terminator).
template< int N, typename HeadT>
struct _TTUnwind<N, HeadT> {
    template<typename ... AspectTs>
    static void set_tuple_aspect_targets(iAbstractValue * trg, std::tuple<AspectTs * ...> t) {
        _Unwind<HeadT>::Doer::set( std::get<N>(t), trg );
    }
    // ...
};

template<typename ... AspectTs> void
unwind_set_targets( iBaseValue<AspectTs...> * t ) {
    aux::_TTUnwind<0, AspectTs ... > \
            ::set_tuple_aspect_targets( t, t->aspects() );
}

template<> inline void
unwind_set_targets<>( iBaseValue<> * ) { /* do nothing when no aspects available */ }

}  // namespace aux
}  // namespace aspects

template<typename ... AspectTs>
iBaseValue<AspectTs ...>::iBaseValue( AspectTs * ... aspects ) : _aspects(aspects ...) {
    aspects::aux::unwind_set_targets( this );
}

template<typename ... AspectTs>
iBaseValue<AspectTs ...>::iBaseValue( std::tuple<AspectTs * ...> t ) : _aspects(t) {
    aspects::aux::unwind_set_targets( this );
}

template<typename ValueT, typename ... AspectTs>
class TValue;

// TODO: unwind aspects array for (before/after)_set advice(s).
template<typename ... AspectTs>
struct Pointcuts {
    //
    // "set" join point
    template<typename ValueT>
    static void before_set( const ValueT & v
                          , TValue<ValueT, AspectTs...> * self ) {}

    template<typename ValueT>
    static void after_set( const ValueT & v
                         , TValue<ValueT, AspectTs...> * self ) {}
};

/**@brief Template container for a value.
 * @class iTValue
 *
 * Represents a value-keeping aspect of the parameter classes.
 * */
template<typename ValueT, typename ... AspectTs>
class TValue : public mixins::iDuplicable< iAbstractValue
                                        , TValue<ValueT, AspectTs...>
                                        , iBaseValue<AspectTs...> > {
public:
    typedef iAbstractValue Base;
    typedef ValueT Value;
    typedef mixins::iDuplicable< iAbstractValue
                               , TValue<ValueT, AspectTs...>
                               , iBaseValue<AspectTs...> > DuplicableParent;
    /// Handler for mutating procedures.
    struct ModificationSeeker {
    public:
        typedef TValue<ValueT, AspectTs...> Owner;
    protected:
        ModificationSeeker( Owner * owner_ ) : owner(*owner_) {
            Pointcuts<AspectTs...>::before_set( owner._mutable_value(), &owner );
        }
    public:
        /// Dft ctr forbidden.
        ModificationSeeker() = delete;
        /// Copy ctr forbidden.
        ModificationSeeker(const ModificationSeeker &) = delete;
        /// Reference to owner parameter entry.
        Owner & owner;
        /// Dtr invoking aspect's advice.
        ~ModificationSeeker(){
            Pointcuts<AspectTs...>::after_set( owner._mutable_value(), &owner ); }
    };
private:
    Value _value;
protected:
    /// Mutable value reference getter. Does NOT invokes aspect's after_set()
    /// advices.
    ValueT & _mutable_value() {
        return _value;
    }
    virtual const std::type_info & _V_target_type_info() const override {
        return typeid(ValueT); }
    /// Sets the option value. Must not be used in ordinary routines as it
    /// does ignore the potential participant(s) from aspects (e.g. IsSet).
    virtual void _set_value( const ValueT & v );
    /// Returns kept value address.
    virtual void * _V_data_addr() override { return &_value; }
    /// Returns kept value address (const).
    virtual const void * _V_data_addr() const override { return &_value; }
    /// Potentially dangerous ctr leaving the value uninitialized.
    TValue() : _value() {}
public:
    template<typename ... ArgTs> TValue(ArgTs...args);
    /// Const value getter (public).
    virtual const ValueT & value() const { return _value; }
    /// Value getter.
    ModificationSeeker && mutable_value() { return ModificationSeeker(this); }
    /// Setter.
    virtual void value(const Value & v) { _set_value(v); }
    /// Ctr "from value". Ignores joint point.
    explicit TValue( const ValueT & v ) : _value(v) {}
    /// Copy ctr. Besides from implicit iDuplicable parent copy invocation,
    /// directly copies value.
    TValue( const TValue<Value> & o ) : _value(o._value) {}
};

template<typename ValueT, typename ... AspectTs>
template<typename ... ctrArgTs>
TValue<ValueT, AspectTs...>::TValue( ctrArgTs ... args ) : DuplicableParent(args ... ) {
}

template< typename ValueT
        , typename ... AspectTs> void
TValue<ValueT, AspectTs...>::_set_value( const ValueT & v ) {
    Pointcuts<AspectTs...>::before_set( v, this );
    this->_mutable_value() = v;
    Pointcuts<AspectTs...>::after_set( v, this );
}

}  // namespace dict
}  // namespace goo

# endif // H_GOO_PARAMETERS_VALUES_H

