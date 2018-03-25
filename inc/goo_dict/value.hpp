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

# include "goo_dict/asp_unwind.tcc"

# include <cassert>

# if !defined(_Goo_m_DISABLE_DICTIONARIES)

namespace goo {
namespace dict {

/**@brief Abstract base for instances within goo::dict.
 * @class iReferable
 *
 * This base expresses the most basic idea behind every entity within the
 * goo::dict framework by declaring a class that refers to physical data block.
 * No assumptions about the particular data structure or type are made here.
 *
 * One may use this class as base for any referable template classes for
 * goo::dict dictionaries.
 * */
class iReferable {
protected:
    /// Shall return untyped data pointer.
    virtual void * _V_data_addr() = 0;
    /// Shall return untyped (const) data pointer.
    virtual const void * _V_data_addr() const = 0;
    /// Shall return C++ RTTI type info.
    virtual const std::type_info & _V_target_type_info() const = 0;  // return typeid()
public:
    iReferable() = default;
    /// Returns untyped data pointer.
    void * data_addr() { return _V_data_addr(); }
    /// Returns untyped (const) data pointer.
    const void * data_addr() const { return _V_data_addr(); }
    /// Returns C++ RTTI value type info.
    virtual const std::type_info & target_type_info() const
                { return _V_target_type_info(); }
};  // class iEnumerable

///\macro _Goo_m_dict_implement_generic_dealer
///\brief Implements default Traits<X>::Dealer template.
///
/// This macro steers default conversion rules for target type, based on
/// ordinary C++ conversion mechanic. To allow ser to define more complex
/// conversions, this class must be specialized.
# define _Goo_m_dict_implement_generic_dealer(X)                            \
    /**\tparam RealT type of value kept by wrapper (see ReferableWrapper) */ \
    /**\tparam DesiredT target conversion type */                           \
    template< typename RealT                                                \
            , typename DesiredT                                             \
            > struct Dealer {                                               \
        static constexpr bool able =                                        \
                std::is_convertible<X<RealT> &, DesiredT>::value;           \
        typedef typename std::conditional<able, DesiredT, void>::type       \
                                                        actual_ret_type;    \
        static actual_ret_type obtain( X<RealT> & t ) { return t; }         \
    };  // Traits<X>::Dealer (generic)

///\macro _Goo_m_dict_implement_value_wrapper
///\brief Implements default Traits<X>::ReferableWrapper template.
///
/// This macro implements default shimmering template class providing access
/// to various value conversions. It is a routing point to forward value
/// handling invocations to various dealer classes (which are merely the
/// strategy pattern). User may want to re-define this behaviour in some rare
/// cases too (do it via the template specialization).
# define _Goo_m_dict_implement_generic_value_wrapper(X)                     \
    template< typename ValueT>                                              \
    struct ReferableWrapper : public iReferable {                           \
    private:                                                                \
        X<ValueT> _v;                                                       \
    protected:                                                              \
        virtual void * _V_data_addr() override { return &_v; }              \
        virtual const void * _V_data_addr() const override { return &_v; }  \
        virtual const std::type_info & _V_target_type_info() const override \
                                            { return typeid(*this); }       \
    public:                                                                 \
        template<typename ... ArgTs> ReferableWrapper( ArgTs ... ctrArgs )  \
                                                        : _v(ctrArgs...) {} \
                                                                            \
        template<typename DesiredT>                                         \
        typename std::enable_if< ReferableTraits<X>                         \
                            ::template Dealer<ValueT, DesiredT>::able       \
                    , typename ReferableTraits<X>                           \
                            ::template Dealer<ValueT, DesiredT>::actual_ret_type \
                    >::type                                                 \
            as() {                                                          \
                return ReferableTraits<X>::template Dealer<ValueT, DesiredT> \
                ::obtain(_v); }                                              \
        X<ValueT> & container() { return _v; }                               \
        const X<ValueT> & container() const { return _v; }                   \
    };  // Traits<X>::ReferableWrapper (generic)

/// Generic traits for goo::dict's entries.
template< template<typename> class X >
struct ReferableTraits {
    _Goo_m_dict_implement_generic_dealer(X);
    _Goo_m_dict_implement_generic_value_wrapper(X);
};


}  // namespace dict
}  // namespace goo

# endif  // !defined(_Goo_m_DISABLE_DICTIONARIES)
# endif  // H_GOO_PARAMETERS_VALUES_H

