# ifndef H_GOO_UTILITY_CPP_H
# define H_GOO_UTILITY_CPP_H

/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# include "goo_types.h"

# include <string>
# include <type_traits>
# include <thread>
# include <mutex>
# include <chrono>
# include <map>
# include <vector>
# include <cmath>

namespace std {
template<bool B, typename T = void> using disable_if = enable_if<!B, T>;
}

namespace stdE {

template<typename charT, typename traits = std::char_traits<charT> >
struct center_helper {
    std::basic_string<charT, traits> str_;
    center_helper(std::basic_string<charT, traits> str) : str_(str) {}
};

template<typename charT, typename traits = std::char_traits<charT> >
center_helper<charT, traits> centered(std::basic_string<charT, traits> str);

template<typename charT, typename traits = std::char_traits<charT> >
std::basic_ostream<charT, traits>& operator<<(std::basic_ostream<charT, traits>& s,
                                              const center_helper<charT, traits>& c);

//
// http://flamingdangerzone.com/ {
//

template <bool B, typename...>
struct dependent_bool_type : std::integral_constant<bool, B> {};
// and an alias, just for kicks :)
template <bool B, typename... T>
using Bool = typename dependent_bool_type<B, T...>::type;

template <typename If, typename Then, typename Else>
using Conditional = typename std::conditional<If::value, Then, Else>::type;

// Meta-logical negation
template <typename T>
using Not = Bool<!T::value>;

// Meta-logical disjunction
template <typename... T>
struct any : Bool<false> {};
template <typename Head, typename... Tail>
struct any<Head, Tail...> : Conditional<Head, Bool<true>, any<Tail...>> {};

// Meta-logical conjunction
template <typename... T>
struct all : Bool<true> {};
template <typename Head, typename... Tail>
struct all<Head, Tail...> : Conditional<Head, all<Tail...>, Bool<false>> {};

namespace detail {
    enum class enabler {};
}

constexpr detail::enabler dummy = {};

template <typename... Condition>
using EnableIf = typename std::enable_if<all<Condition...>::value, detail::enabler>::type;

//template <typename Condition, typename T = void>
//using EnableIf = typename std::enable_if<Condition::value, T>::type;

template <typename Condition, typename T = void>
using DisableIf = typename std::enable_if<!Condition::value, T>::type;

//
// }
//

//
// Thanks to http://stackoverflow.com/questions/9851594/standard-c11-way-to-remove-all-pointers-of-a-type {
// NOTE: do not removes extents ([]). Use std::remove_all_extents<>.

template <typename T> struct remove_all_pointers{
    typedef T type;};

template <typename T> struct remove_all_pointers<T*>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<T* const>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<T* volatile>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<T* const volatile >{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<T[]>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T, size_t N> struct remove_all_pointers<T[N]>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<const T[]>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T, size_t N> struct remove_all_pointers<const T[N]>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T> struct remove_all_pointers<volatile T[]>{
    typedef typename remove_all_pointers<T>::type type;};

template <typename T, size_t N> struct remove_all_pointers<volatile T[N]>{
    typedef typename remove_all_pointers<T>::type type;};

//
// }
//

template< class T > struct remove_dimension                     {typedef T type;};
template< class T > struct remove_dimension<T*>                 {typedef T type;};
template< class T > struct remove_dimension<T* const>           {typedef T type;};
template< class T > struct remove_dimension<T* volatile>        {typedef T type;};
template< class T > struct remove_dimension<T* const volatile>  {typedef T type;};
template< class T > struct remove_dimension<T[]>                {typedef T type;};
template< class T > struct remove_dimension<const T[]>          {typedef T type;};
template< class T > struct remove_dimension<volatile T[]>       {typedef T type;};
template< class T > struct remove_dimension<const volatile T[]> {typedef T type;};
template< class T, std::size_t N > struct remove_dimension<T[N]>                {typedef T type;};
template< class T, std::size_t N > struct remove_dimension<const T[N]>          {typedef T type;};
template< class T, std::size_t N > struct remove_dimension<volatile T[N]>       {typedef T type;};
template< class T, std::size_t N > struct remove_dimension<const volatile T[N]> {typedef T type;};

//
//
//

template<class T>
struct ptr_rank : std::integral_constant<std::size_t, 0> {};


template<class T>
struct ptr_rank<T*> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T>
struct ptr_rank<T[]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T, size_t N>
struct ptr_rank<T[N]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};


template<class T>
struct ptr_rank<const T*> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T>
struct ptr_rank<const T[]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T, size_t N>
struct ptr_rank<const T[N]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};


template<class T>
struct ptr_rank<const volatile T*> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T>
struct ptr_rank<const volatile T[]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T, size_t N>
struct ptr_rank<const volatile T[N]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};


template<class T>
struct ptr_rank<volatile T*> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T>
struct ptr_rank<volatile T[]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template<class T, size_t N>
struct ptr_rank<volatile T[N]> : public std::integral_constant<size_t, ptr_rank<T>::value + 1> {};

template <typename...>
struct is_one_of {
    static constexpr bool value = false;
};

/// Has value=true, if first given type is one of the rest.
template <typename F, typename S, typename... T>
struct is_one_of<F, S, T...> {
    static constexpr bool value =
            std::is_same<F, S>::value || is_one_of<F, T...>::value;
};

// Filters parameters pack

template <typename, typename> struct Cons;

template <typename  T, typename ...Args>
struct Cons<T, std::tuple<Args...> > {
    using type = std::tuple<T, Args...>;
};

template <template <typename> class Predicate, typename...> struct filter;

template <template <typename> class Predicate> struct filter<Predicate> {
    using type = std::tuple<>;
};

template< template <typename> class Predicate
        , typename Head
        , typename ...Tail>
struct filter<Predicate, Head, Tail...> {
    using type = typename std::conditional< Predicate<Head>::value
                                          , typename Cons<Head, typename filter<Predicate, Tail...>::type>::type
                                          , typename filter<Predicate, Tail...>::type
                                          >::type;
};

} // namespace stdE

namespace goo {

/// concatenates two multiline strings into one table
std::string concat_block_strings( const std::string & s1,
                                  const std::string & s2 );

/// fwrite() with exception on error shortcut
void cf_write( FILE *, Size, const void * );

/// fread() with exception on error shortcut
void cf_read( FILE *, Size, void * );

/// Uses printf()-like syntax to produce std::string in one-line expr.
std::string strfmt( const char * fmt, ... );

/// A logging stream class, convenient for parallel applications.
class ParStream {
protected:
    typedef std::chrono::high_resolution_clock::time_point TP;
    std::mutex _mtx;
    std::multimap<TP, std::pair<UByte, std::string> > _log;
    unsigned char _nCols;
    unsigned short * _nMaxChars;
public:
    ParStream( unsigned char ncols ) :
        _nCols(ncols+1),
        _nMaxChars(new unsigned short [_nCols]) {
        for( unsigned char i = 0; i < _nCols; ++i ) {
            _nMaxChars[i] = 1;
        }
    }
    ~ParStream() { delete [] _nMaxChars; }
    void push( unsigned char ncol, const std::string & msg );
    void dump( std::ostream & );
};

/// Splits a STL-complaint container content between siblings.
template<typename ContainerT> void
split_sequence( const ContainerT & src,
                ContainerT * targets[],
                std::size_t n ) {
    std::size_t step = ceil( double(src.size())/double(n) );
    auto srcEndIt = src.cbegin(),
         srcBgnIt = src.cbegin();
    for( std::size_t i = 0; i < n-1; ++i ) {
        ContainerT & target = *targets[i];
        target.reserve(step);
        std::advance(srcEndIt, step);
        std::copy( srcBgnIt, srcEndIt, std::inserter(target, target.begin()));
        std::advance(srcBgnIt, step);
    }
    ContainerT & last = *targets[n-1];
    std::copy( srcBgnIt, src.cend(), std::inserter(last, last.begin()));
}

template<typename T> constexpr typename std::enable_if<std::is_fundamental<T>::value, TypeCode>::type
encode_type();  // default be undefined by design.

#define declare_cexpr_ftype_encoder( num, cnm, hnm, hdsnm ) \
template<> constexpr TypeCode encode_type<cnm>() { return num; }
for_all_atomic_datatypes(declare_cexpr_ftype_encoder)
# undef declare_cexpr_ftype_encoder


// May be useful in tuple-to-signature argument expansion. See:
// https://habrahabr.ru/post/228031/ (rus)
// http://stackoverflow.com/questions/7858817/unpacking-a-tuple-to-call-a-matching-function-pointer
# if 0
template<typename FT,
         typename TupleT,
         bool EnoughT,
         size_t TotalArgsT,
         size_t ... NT>
struct CallImpl {
    static auto call(FT f, TupleT && t) {
        return CallImpl<FT, TupleT, TotalArgsT == 1 + sizeof...(NT),
                                    TotalArgsT, NT ..., sizeof...(NT)
                        >::call(f, std::forward<TupleT>(t));
    }
};

template<typename FT,
         typename TupleT,
         int TotalArgsT,
         int... NT>
struct CallImpl<FT, TupleT, true, TotalArgsT, NT...> {
    auto static call(FT f, TupleT && t) {
        return f(std::get<NT>(std::forward<TupleT>(t))...);
    }
};
# endif

}  // namespace goo

# endif  // H_GOO_UTILITY_CPP_H

