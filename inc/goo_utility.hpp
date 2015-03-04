# ifndef H_GOO_UTILITY_CPP_H
# define H_GOO_UTILITY_CPP_H

# include <string>
# include <type_traits>
# include <thread>
# include <mutex>
# include <chrono>
# include <map>
# include <vector>
# include "goo_mixins.tcc"

namespace std {
template<bool B, typename T = void> using disable_if = enable_if<!B, T>;
};

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

} // namespace stdE

namespace goo {

/// concatenates two multiline strings into one table
std::string concat_block_strings( const std::string & s1,
                                  const std::string & s2 );

/// fwrite() with exception on error shortcut
void cf_write( FILE *, Size, const void * );

/// fread() with exception on error shortcut
void cf_read( FILE *, Size, void * );

template<typename T>
T closest_value(std::vector<T> const& vec, T value) {
    auto it = std::lower_bound(vec.begin(), vec.end(), value);
    if(it == vec.end()) {  // all elements are less than ``value''
        return *vec.crbegin();
    } else if( it == vec.cbegin() ) {  // all elements are greater then ``value''
        return *it;
    }
    const T & right = *it,
            & left  = *(--it);
    T distanceBefore = value - left,
      distanceAfter  = right - value;
    return ( distanceBefore > distanceAfter ? right : left );
}

/// Uses printf()-like syntax to produce std::string in one-line expr.
std::string strfmt( const std::string & fmt, ... );

/// A logging stream class, convinient for parallel applications.
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
encode_type() {
    emraise(badCast, "Provided type \"%s\" is unsupported.", typeid(T).name() ); }

#define declare_cexpr_ftype_encoder( num, cnm, hnm, hdsnm ) \
template<> constexpr TypeCode encode_type<cnm>() { return num; }
for_all_atomic_datatypes(declare_cexpr_ftype_encoder)
# undef declare_cexpr_ftype_encoder

}  // namespace goo

# endif  // H_GOO_UTILITY_CPP_H

