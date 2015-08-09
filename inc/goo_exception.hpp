# ifndef HPH_EXCEPTION_HPP
# define HPH_EXCEPTION_HPP

# include "goo_types.h"
# include "goo_ansi_escseq.h"
# include "goo_utility.h"

# ifdef __cplusplus
#   include <string>
#   include <exception>
#   ifdef EM_STACK_UNWINDING
#       include <forward_list>
#       ifndef NO_BFD_LIB
#           include <bfd.h>
#           include <link.h>
#       endif
#   endif
#   ifdef NO_BFD_LIB
    /// When no bfd defined, alias type as this.
    typedef unsigned long long bfd_vma;
#   endif
# else
#   include <string.h>
#   include <stdio.h>
#   include <stdarg.h>
# endif

/*! \addtogroup errors
 *
 * Goo use exceptions as an general error reporting mechanism.
 * Exceptions on various build configurations provides a way
 * to extract a stacktrase information even when no debug
 * info is given (in that case this info is however rather
 * uninformative). To disable stactrace set the EM_STACK_UNWINDING cmake
 * option to OFF.
 *
 * Each exception reason is encoded by unique number from enum.
 * Each number has a generic brief and user-specific textual
 * description that usually provides a detailed description.
 *
 * Exceptions are thrown with emraise() macro with variadic
 * arguments and ANSI printf() format string.
 *
 * There are three developments shortcuts:
 * _TODO_, _UNSUPPORTED_, _FORBIDDEN_CALL_.
 *
 * There are also several helper macro providing standard way to
 * report things:
 *  * dprintf() -- to print a debug info (disabled when NDEBUG macro provided)
 *  * warning() -- to print a warning.
 *  * eprintf() -- to print an error without throwing an exception.
 * Each macro shortcut uses ANSI printf() syntax and have buffer length
 * limited to 512 bytes. When NDEBUG definition is not defined each
 * message from that shortcut will be prefixed with info about
 * place where it was invoked: source file, line and current routine
 * name and signature.
 */

/*!\def emraise
 * \brief Throws the goo::Exception object with detailed description.
 * \ingroup errors
 *
 * Format is standard for ANSI printf() family.
 * \param c is a encoded generic error ID from for_all_errorcodes macro.
 */
# define emraise( c, ... ) while(true){ \
    char bf[EMERGENCY_BUFLEN]; \
    snprintf(bf, EMERGENCY_BUFLEN, __VA_ARGS__ ); \
    throw goo::Exception((int) goo::Exception::c, bf );}

/*!\def _TODO_
 * \brief Development helper macro to raise 'unimplemented' error.
 * \ingroup errors */
# define _TODO_ emraise( unimplemented, "%s:%d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__ )
/*!\def _UNSUPPORTED_
 * \brief Development helper macro to raise 'unsupported' error.
 * \ingroup errors
 *
 * Used for cases when this routine is not avalible in current build
 * configuration. */
# define _UNSUPPORTED_ emraise( unsupported, "%s:%d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__ )
/*!\def _FORBIDDEN_CALL_
 * \ingroup errors
 * \brief Development macro to be thrown from forbidden ctrs.
 * Indicates bad architecture (badArchitect). */
# define _FORBIDDEN_CALL_ emraise( badArchitect, "(forbidden call) %s:%d %s", __FILE__, __LINE__, __PRETTY_FUNCTION__ )

/*!\def eprintf
 * \brief Prints error message to standard Hph's error stream.
 * \ingroup errors */
# ifdef PRINTF_SOURCE_INFO
# define eprintf( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDRED "[E%7s]" ESC_CLRCLEAR " at %s:%d %s", hctime(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# else
# define eprintf( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDRED "[E%7s]" ESC_CLRCLEAR " ", hctime() ); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# endif

/*!\def dprintf
 * \brief Prints debug message to standard Hph's error stream.
 * Enabled only in debug builds.
 * \ingroup errors */
# ifndef NDEBUG
# ifdef PRINTF_SOURCE_INFO
# define dprintf( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDCYAN "[D%7s]" ESC_CLRCLEAR " at %s:%d %s", hctime(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# else  // PRINTF_SOURCE_INFO
# define dprintf( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDCYAN "[D%7s]" ESC_CLRCLEAR " ", hctime() ); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# endif // PRINTF_SOURCE_INFO
# else
# define dprintf( ... ) ((void)(0));
# endif

/*!\def warning
 * \brief Prints warn message to standard Hph's error stream.
 * \ingroup errors */
# ifdef PRINTF_SOURCE_INFO
# define warning( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDYELLOW "[W%7s]" ESC_CLRCLEAR " at %s:%d %s", hctime(), __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# else
# define warning( ... ) while(1) {      \
    char  bf[256];                      \
    char  prfxBf[512];                  \
    snprintf(prfxBf, 256, ESC_BLDYELLOW "[W%7s]" ESC_CLRCLEAR " ", hctime() ); \
    snprintf(bf, 256, __VA_ARGS__ );    \
    strcat( bf, ESC_CLRCLEAR );         \
    strcat( prfxBf, bf );               \
    fputs(prfxBf, stderr);              \
    break;                              \
};
# endif

# ifdef __cplusplus

namespace goo {
namespace em {

# ifndef ENABLE_ALLOCATORS
typedef std::string String;
# else
# error "Allocators subsystem is supported, but actual emString implementation is still unimplemented."
# endif

# ifdef EM_STACK_UNWINDING
struct StackTraceInfoEntry {
    unsigned int lFound;        ///< 1 if line was found, 0 otherwise.
    unsigned int lineno;        ///< line number
    String      function,       ///< function name (possibly, mangled)
                elfFilename,    ///< object filename
                srcFilename,    ///< source filename
                failure;        ///< failure reason
# ifndef NO_BFD_LIB
    bfd_vma     addr,           ///< runtime ELF address
                soLibAddr;      ///< static shared library ELF address
    asymbol **  sTable;
# endif
};
typedef std::forward_list<StackTraceInfoEntry> List; // TODO

std::ostream & operator<<(std::ostream& os, const StackTraceInfoEntry & t);

# endif  // EM_STACK_UNWINDING

}  // namespace em (emergency)

/*!\class Exception
 * \ingroup errors
 * \brief Haephestus exception instance.
 *
 * Haephestus' own Exception class that holds following descriptive info:
 * error code (see for_all_errorcodes macro).
 *
 * Use dump() to print all information carried by instance.
 *
 * Inherited from std::exception.
 */
class Exception : public std::exception {
public:
    # define declare_static_const(num, nm, dscr) \
        static const ErrCode nm;
    for_all_errorcodes( declare_static_const )
    # undef declare_static_const
protected:
    ErrCode       _code;
    em::String    _what;
    # ifdef EM_STACK_UNWINDING
    em::List      _stacktrace;
    void _get_trace() throw();
    # endif
public:
    /// Main (recommended) constructor.
    Exception(  const ErrCode       c=0,
                const em::String & s="");
    /// Primitive constructor for most generic exception.
    Exception( const em::String & s="" );

    virtual ~Exception( ) throw();

    /// Hph's error code getter.
    inline ErrCode code() const { return _code; }
    /// Generic method for printing a brief info.
    virtual const char * what() const throw() { return _what.c_str(); }

    /// Generic method for printing a full info.
    void dump(std::ostream &) const throw();
};  // class Exception

namespace em {
String demangle_class( const char * );
String demangle_function( const String & name );
}  // namespace em

}  // namespace goo

# endif /* __cplusplus */

# ifdef __cplusplus
extern "C" {
# endif

/**@brief raises custom HPH-exception from c-code
 *
 * C-function with C++ linkage that throws HPH-exception.
 * Not defined for C++ code.
 */
extern int C_error( ErrCode, const char * fmt, ... );

# ifdef __cplusplus
} // extern "C"
# endif


/*! @} */

# endif  /* HPH_EXCEPTION_HPP */

