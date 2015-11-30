# ifndef GOO_TYPES_H
# define GOO_TYPES_H

# include <stdint.h>
# include "goo_config.h"
# ifdef __cplusplus
# include <string>
# endif

/*! \addtogroup types
 *
 * Here are some shortcuts for simple types used in projects.
 * Such naming often helps code to be more laconic and
 * flexible.
 * @{
 */

/*! \file goo_types.h
 * \brief Contains atomic Goo's types and definitions.
 * 
 * Note the following types table:
 *  #   C-name b0000-0000  hex#
 *  1.    int8  0000 1001  0x9
 *  2.   uint8  0000 0001  0x1
 *  3.   int16  0001 1001  0x19
 *  4.  uint16  0001 0001  0x11
 *  5.   int32  0010 1001  0x29
 *  6.  uint32  0010 0001  0x21
 *  7.   int64  0011 1001  0x39
 *  8.  uint64  0011 0001  0x31
 *  7.  int128  0111 1001  0x79
 *  8. uint128  0111 0001  0x71
 *
 * 10.   float  0000 0010  0x2
 * 11.  double  0000 1010  0xa
 * 12. ldouble  0001 0010  0x12
 * 12.lldouble  0001 1010  0x1a
 *
 * 13.   logic  0000 1100  0xc
 *
 * 14.  string  1000 0011  0x83
 *
 * 15.  struct  1000 0100  0x84
 *
 * The type code numbers are composed in such a way, that one
 * could expose some supplementary info about type with expressions:
 *
 *          IS_INTEGRAL_TYPE() -- true, if type is integral
 *   IS_SIGNED_INTEGRAL_TYPE() -- true, if type is signed integral
 *    IS_FLOATING_POINT_TYPE() -- true, if type is floating point
 *             IS_LOGIC_TYPE() -- true, if type is logic
 *          IS_COMPOUND_TYPE() -- true, if type is compound (e.g. non-atomic)
 **/

# define          IS_INTEGRAL_TYPE(tcode)   ( (tcode & 0x1) && !(tcode & 0x2) )
# define   IS_SIGNED_INTEGRAL_TYPE(tcode)   (!(tcode & 0x8) && IS_INTEGRAL_TYPE(tcode))
# define    IS_FLOATING_POINT_TYPE(tcode)   ( (tcode & 0x2) && !(tcode & 0x1) )
# define             IS_LOGIC_TYPE(tcode)   ( tcode == 0xc )
# define          IS_COMPOUND_TYPE(tcode)   ( 1 && (tcode & 0x80) )

# if 0
/** Not-a-number macro when it isn't defined. */
# ifndef NaN
#   ifndef NAN
#       define NaN (0./.0)
#   else
#       define NaN NAN
#   endif
# endif
# endif


/*!\def GOO_SIZE_TYPE
 * \brief an unsigned 8-byte integer type for
 * data serialization.
 */
# if __x86_64__
#  define GOO_SIZE_TYPE unsigned long
#  define GOO_SIZE_FMT "%ld"
#  else
#  define GOO_SIZE_TYPE unsigned long long
#  define GOO_SIZE_FMT "%lld"
# endif

/*
 * Types X-macro
 */

/*! \def for_all_atomic_datatypes
 *  \brief macro includes all atomic Goo's data types information
 * 
 * By 'atomic' we mean here 'fixed length' data types. X-macro definition
 * helps to arrange type information in convinient table view.
 *
 * Depending on build configuration choosen, this macro can contain additional
 * data types like 128-bit ones.
 */

# ifndef TYPES_128BIT_LENGTH
# define for_all_atomic_datatypes(m)                    \
    m(0x9,           uint8_t,   UByte,      UInt8     ) \
    m(0x1,            int8_t,   SByte,      Int8      ) \
    m(0x19,         uint16_t,   UShort,     UInt16    ) \
    m(0x11,          int16_t,   SShort,     Int16     ) \
    m(0x29,         uint32_t,   UInt,       UInt32    ) \
    m(0x21,          int32_t,   SInt,       Int32     ) \
    m(0x39,         uint64_t,   ULong,      UInt64    ) \
    m(0x31,          int64_t,   SLong,      Int64     ) \
    m(0x2,             float,   Float4,     Float32   ) \
    m(0xa,            double,   Float8,     Float64   ) \
    /* ... */

# else  /* TYPES_128BIT_LENGTH */
# define for_all_atomic_datatypes(m)                    \
    m(0x9,           uint8_t,   UByte,      UInt8     ) \
    m(0x1,            int8_t,   SByte,      Int8      ) \
    m(0x19,         uint16_t,   UShort,     UInt16    ) \
    m(0x11,          int16_t,   SShort,     Int16     ) \
    m(0x29,         uint32_t,   UInt,       UInt32    ) \
    m(0x21,          int32_t,   SInt,       Int32     ) \
    m(0x39,         uint64_t,   ULong,      UInt64    ) \
    m(0x31,          int64_t,   SLong,      Int64     ) \
    m(0x79,        uint128_t,   ULLong,     UInt64    ) \
    m(0x71,         int128_t,   SLLong,     Int64     ) \
    m(0x2,             float,   Float4,     Float32   ) \
    m(0xa,            double,   Float8,     Float64   ) \
    m(0x12,      long double,   Float16,    Float128  ) \
    /* ... */
# endif /* TYPES_128BIT_LENGTH */

/*!\def for_all_errorcodes
 * \ingroup errors
 * \brief xmacro defining a table of error codes.
 *
 * We've tried to deduce most regular types of error
 * types in this table to describe most generic error
 * reasons. Despite the rather free nature of this
 * table it has proved itself as quite convinient
 * approach.
 *
 * This cmacro table is used to reporting errors in C++
 * and in ANSI C part of the Goo library.
 */
# define for_all_errorcodes(m) \
    m(1,        common,             "unspecific" ) \
    m(2,        unimplemented,      "unimplemented routine reached") \
    m(3,        unsupported,        "feature is unsupported in current build configuration") \
    m(4,        uTestFailure,       "unit test failed" ) \
    m(5,        interpreter,        "general interpreter error" ) \
    m(6,        nullPtr,            "got null pointer" )\
    m(7,        unknownType,        "unknown type") \
    m(8,        badCast,            "incompatable types" ) \
    m(9,        malformedArguments, "malformed arguments" ) \
    m(10,       badState,           "bad state for such conditions") \
    m(11,       noSuchKey,          "key not found" ) \
    m(12,       nonUniq,            "non-uniq entry offered" ) \
    m(13,       memAllocError,      "memory depleted" ) \
    m(14,       lenMismatch,        "length is inadequate" )\
    m(15,       underflow,          "underflow index met" ) \
    m(16,       overflow,           "overflow index met" ) \
    m(17,       narrowConversion,   "narrow number conversion is possible") \
    m(18,       uninitialized,      "operation invoked for uninitialized data") \
    m(25,       threadError,        "execution stopped due to in-thread error") \
    m(50,       nwGeneric,          "network error") \
    m(100,      ioError,            "common I/O error") \
    m(101,      notFound,           "data is not found at location") /* TODO: rename to noSuchData */ \
    m(102,      corruption,         "malformed data") \
    m(103,      badValue,           "got unacceptable numeric value") \
    m(104,      fileNotReachable,   "file is unreachable") \
    m(105,      badParameter,       "got invalid parameter or parameter set" ) \
    m(106,      objNotConstructed,  "requested object was not constructed" ) \
    m(107,      singletonRepCtr,    "singleton repeated construction" ) \
    m(108,      badArchitect,       "architectural incompleteness or undefined state" ) \
    m(254,      gdsError,           "Declarative Semantics language parsing error") \
    m(255,      thirdParty,         "thirt-party code error") \
    /* ... */

/*!\def for_all_statuscodes
 * \ingroup errors
 * \brief xmacro defining a table of status codes.
 *
 * Includes all error codes + 0 code of success result.
 */
# define for_all_statuscodes(m) \
    m(0,        success,             "Success." ) \
    for_all_errorcodes(m) \
    /* ... */

/*
 * Declare types
 */

# define declare_typedef( num, cnm, hnm, gdsnm ) \
    typedef cnm hnm;
for_all_atomic_datatypes(declare_typedef)
# undef declare_typedef

typedef uint8_t         ErrCode;    ///< Goo error code type.
typedef GOO_SIZE_TYPE   Size;       ///< Goo size type.
typedef uint8_t         TypeCode;   ///< goo type descriptor code.

const char * get_errcode_description( const ErrCode C );

# ifdef __cplusplus
extern "C" {
# endif

/**@brief returns textual name of the atomic data type by its code
 *
 * Uses internal static array with textualized type names for atomic
 * types.
 *
 * Returns "void" if type code is invalid.
 **/
const char * get_atomic_type_name( TypeCode );

/**@brief returns size of an atomic type in bytes
 *
 * Uses internal static array with sizes for atomic
 * types.
 *
 * Returns 0 if type code is invalid.
 **/
UByte get_atomic_type_size( TypeCode );

# ifdef __cplusplus
}
# endif

/*
 * Class Shortcuts
 */

# ifdef __cplusplus

/*! \def DECLTYPE
 *  \brief type substitution shortcut.
 *
 *  E.g. for:
 *  \code{.cpp}
 *  std::vector<int> vec;
 *  ...
 *  DECLTYPE(vec)::iterator it = vec.begin();
 *  std::vector<int>::iterator it = vec.end();
 *  \endcode
 *  the last two lines are equivalent. The macro is particularly
 *  useful for ling template types e.g.
 *  <tt>std::vector&lt;std::map&lt;std::string, std::pair&lt;std::string, int&gt; &gt; &gt;</tt>.
 */
template<typename T> struct decltype_t { typedef T type; };
#define DECLTYPE(expr) decltype_t<decltype(expr)>::type

namespace goo {

typedef uint8_t AtomicTypeID;

template<typename T> AtomicTypeID
get_atomic_typeid() { return 0; }

# define declare_typeid_getter( num, cnm, hnm, gdsnm ) \
template<> AtomicTypeID get_atomic_typeid< cnm >();
for_all_atomic_datatypes(declare_typeid_getter)
# undef declare_typeid_getter

}  // namespace goo

# endif  // __cplusplus

/*
 * Reflection
 */

# define declare_dtype_introspection_code( num, cType, name, gdsnm ) \
    extern const TypeCode name ## _T_code;
    for_all_atomic_datatypes(declare_dtype_introspection_code)
    # undef declare_dtype_introspection_code

/*! @} */

# endif  /* GOO_TYPES_H */

