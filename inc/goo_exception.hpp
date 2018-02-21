# ifndef H_GOO_EXCEPTION_H
# define H_GOO_EXCEPTION_H

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
# include "goo_ansi_escseq.h"
# include "goo_utility.h"

# include <string.h>

# ifdef __cplusplus
#   include <string>
#   include <exception>
#   include <typeinfo>
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
 * info is given (in that case this info is not, however, rather
 * uninformative). To disable stactrace set the EM_STACK_UNWINDING cmake
 * option to OFF.
 *
 * Each exception reason is encoded by unique number from enum.
 * Each number has a generic brief and user-specific textual
 * description that usually provides a detailed description.
 *
 * Exceptions are thrown with emraise() macro with variadic
 * arguments and ANSI printf() format string. It is a recommended way
 * as emraise internally checks for user reporting system (see below).
 *
 * There are three developments shortcuts:
 * _TODO_, _UNSUPPORTED_, _FORBIDDEN_CALL_.
 *
 * There are also several helper macro providing standard way to
 * report things:
 *  * dprintf() -- to print a debug info (disabled when NDEBUG macro provided)
 *  * wprintf() -- to print a warning.
 *  * eprintf() -- to print an error without throwing an exception.
 * Each macro shortcut uses ANSI printf() syntax and have buffer length
 * limited to 512 bytes. When NDEBUG definition is not defined each
 * message from that shortcut will be prefixed with info about
 * place where it was invoked: source file, line and current routine
 * name and signature.
 *
 * Taking into account that sometimes user desires to trigger its own
 * error-reporting system routines, we provide a way to report an error
 * to user's handler and, depending on its result throw or not a
 * Goo's exception. See Exception::user_raise routine.
 *
 * It's recommended for your code that uses goo::Exception to check
 * the Exception::user_raise() first.
 */

/*!\def emraise
 * \brief Throws the goo::Exception object with detailed description.
 * \ingroup errors
 *
 * Format is standard for ANSI printf() family.
 * \param c is a encoded generic error ID from for_all_errorcodes macro.
 */
# define emraise( c, ... ) while(true){                                        \
    char __goo_em_bf[GOO_EMERGENCY_BUFLEN];                                    \
    snprintf(__goo_em_bf, GOO_EMERGENCY_BUFLEN, __VA_ARGS__ );                 \
    if( goo::Exception::user_raise((int) goo::Exception::c, __goo_em_bf ) ) {  \
        throw goo::TheException<goo::Exception:: c>( __goo_em_bf );}           \
    }

/*!\def emraise_tp
 * \brief Throws the goo::Exception object with detailed description constructed
 *        with appropriate arguments.
 * \ingroup errors
 *
 * Format is standard for ANSI printf() family.
 * \param c is a encoded generic error ID from for_all_errorcodes macro.
 */
# define emraise_tp( c, ... ) while(true){                                     \
    if( goo::Exception::user_raise( (int) goo::Exception::c, "<no details>" ) ) {  \
        throw goo::TheException<goo::Exception:: c>( __VA_ARGS__ );}           \
    }

/*!\def _TODO_
 * \brief Development helper macro to raise 'unimplemented' error.
 * \ingroup errors */
# define _TODO_ emraise_tp( unimplemented                \
                          , __LINE__, __FILE__           \
                          , __PRETTY_FUNCTION__, "(dev)" )

/*!\def _DETAILED_TODO_
 * \brief Development helper macro to raise 'unimplemented' error.
 * \ingroup errors */
# define _DETAILED_TODO_( ... ) while(true){                                   \
    char __goo_em_bf[GOO_EMERGENCY_BUFLEN];                                    \
    snprintf(__goo_em_bf, GOO_EMERGENCY_BUFLEN, __VA_ARGS__ );                 \
    if( goo::Exception::user_raise((int) goo::Exception::unimplemented, __goo_em_bf ) ) {  \
        throw goo::TheException<goo::Exception::unimplemented>(                \
                    __LINE__, __FILE__, __PRETTY_FUNCTION__, __goo_em_bf );}   \
    }

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
 * \brief Prints error message to standard Goo's error stream.
 * \ingroup errors */
# ifdef SOURCE_POSITION_INFO
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
 * \brief Prints debug message to standard Goo's error stream.
 * Enabled only in debug builds.
 * \ingroup errors */
# ifndef NDEBUG
# ifdef SOURCE_POSITION_INFO
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
# else  // SOURCE_POSITION_INFO
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
# endif // SOURCE_POSITION_INFO
# else
# define dprintf( ... ) ((void)(0));
# endif

/*!\def wprintf
 * \brief Prints warn message to standard Goo's error stream.
 * \ingroup errors */
# ifdef SOURCE_POSITION_INFO
# define wprintf( ... ) while(1) {      \
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
# define wprintf( ... ) while(1) {      \
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

# ifndef FNTHN_ALLOCATORS
typedef std::string String;
# else
//#   warning "Allocators subsystem is supported, but actual emString implementation is still unimplemented."
typedef std::string String;  // TODO
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
 * \brief Goo exception instance.
 *
 * Goo's own Exception class that holds following descriptive info:
 * error code (see for_all_errorcodes macro).
 *
 * Use dump() to print all information carried by instance.
 *
 * Inherited from std::exception.
 */
class Exception {
public:
    # define declare_static_const(num, nm, dscr) \
        static constexpr ErrCode nm = num;
    for_all_errorcodes( declare_static_const )
    # undef declare_static_const
    /// User's handler slot. Should return true wher exception throw is approved.
    static bool (*user_raise)( const ErrCode      c,
                               const em::String & s );
    /// Default raise trigger -- always approves exception throw.
    static bool goo_raise( const ErrCode, const em::String & );
protected:
    ErrCode       _code;
    em::String    _what;
    # ifdef EM_STACK_UNWINDING
    em::List      _stacktrace;
    void _get_trace() throw();
    # endif
public:
    /// Generic constructor.
    Exception( const ErrCode       c=0
             , const em::String & s="");
    /// Primitive constructor for most generic exception.
    Exception( const em::String & s="" );

    virtual ~Exception() throw();

    /// Goo's error code getter.
    inline ErrCode code() const { return _code; }
    /// Prints the exception type-specific information in human-readable form.
    virtual void dump_specific_details( std::ostream & ) const throw();
    /// Generic method for printing a brief info.
    virtual const char * what() const throw() { return _what.c_str(); }

    /// Generic method for printing a full info.
    void dump(std::ostream &) const throw();
};  // class Exception

# define GOO_EXCPTN_IMPLEMENT_DFT_CTR(C_M) \
    TheException( const em::String & s="" ) : Exception( C_M, s ) {}

template<ErrCode ECodeT>
class TheException : public Exception
                   , public std::exception {
public:
    GOO_EXCPTN_IMPLEMENT_DFT_CTR(ECodeT)
};

namespace em {
String demangle_class( const char * );
String demangle_function( const String & name );
}  // namespace em


/**\brief Development exception indicating functionality to be implemented.
 * \ingroup errors
 *
 * Albeit such kind of exceptions shall never be met at the production code,
 * the practice of putting them temporary into development procedures is the
 * common practice.
 *
 * This exception specific ctr relies on some information provided ususally by
 * compiler with macro suxh definitions as __FILE__ or __LINE__.
 */
template<>
class TheException<Exception::unimplemented> : public Exception
                                             , public std::exception {
private:
    int _lineNo;
    em::String _fileName, _prettyFunction;
public:
    explicit TheException( const em::String & s="" ) : Exception( Exception::unimplemented, s )
                                                     , _lineNo(-1)
                                                     , _fileName("<not set>")
                                                     , _prettyFunction("<not set>") {}
    TheException( int lNo
                , const em::String & fn
                , const em::String & pf
                , const em::String & description );
    /// Prints the code positional info.
    void dump_specific_details( std::ostream & ) const throw() override;
    // TODO: ... setters/getters
};

/**\brief Exception indicating bad type cast.
 * \ingroup errors
 *
 * The bad dynamic type cast occurs when user code tries to perform wrong type
 * conversion operation: the target type is not a ancestor or descendant of
 * source type. This error is usually indicates that some dynamic data
 * structure(s) was built erroneously due to mistake at the incoming data.
 */
template<>
class TheException<Exception::badCast> : public Exception
                                       , public std::bad_cast {
private:
    const std::type_info * _fromType
                       , * _toType;
    const void * _addr;
public:
    TheException( const em::String & s="" ) : Exception( Exception::badCast, s )
                                            , _fromType( nullptr )
                                            , _toType( nullptr )
                                            , _addr(nullptr) {}

    TheException( const std::type_info * fTI
                , const std::type_info * tTI
                , const void * addr
                , const em::String & s="" ) : Exception( Exception::badCast, s )
                                            , _fromType( fTI )
                                            , _toType( tTI )
                                            , _addr(addr) {}

    template< typename FromTypeT
            , typename ToTypeT>
    static TheException<Exception::badCast> _( const void * addr
                , const em::String & s="" ) throw() {
        return TheException( &typeid(FromTypeT), &typeid(ToTypeT), addr, s );
    }

    /// Prints the code positional info.
    void dump_specific_details( std::ostream & ) const throw() override;

    const std::type_info * from_type_info() const throw() { return _fromType; }
    const std::type_info * to_type_info() const throw() { return _toType; }
    const void * address() const throw() { return _addr; }
};

/*!\def goo_badcast
 * \brief Shortcut macro for throwing goo::Exception's "bad cast" descendant.
 * \ingroup errors
 */
# define goo_badcast( fT, tT, addr, ... ) while(true){                          \
    char bf[GOO_EMERGENCY_BUFLEN];                                              \
    snprintf(bf, sizeof(bf), __VA_ARGS__ );                                     \
    if( goo::Exception::user_raise( (int) goo::Exception::badCast, bf ) ) {     \
        typedef goo::TheException<goo::Exception::badCast> TE;                   \
        throw TE::_<fT, tT>( addr, bf );}    \
    }


template<>
class TheException<Exception::uninitialized> : public Exception
                                             , public std::logic_error {
private:
    em::String _dataLabel;
    const void * _addr;
public:
    TheException( const em::String & s="" ) : Exception( Exception::uninitialized, s )
                                            , std::logic_error(s.c_str())
                                            , _addr(nullptr) {}

    TheException( const em::String & label
                , const void * addr
                , const em::String & s="" ) : Exception( Exception::badCast, s )
                                            , std::logic_error(s.c_str())
                                            , _dataLabel(label)
                                            , _addr(addr) {}

    void dump_specific_details( std::ostream & ) const throw() override;
    const void * address() const throw() { return _addr; }
};

// ... further specifications (add by demand)

# undef GOO_EXCPTN_IMPLEMENT_DFT_CTR

}  // namespace goo

# endif /* __cplusplus */

# ifndef NDEBUG
# ifdef __cplusplus /* C++ --- use exception */
# define DBG_NULLPTR_CHECK( ptr, ... ) \
{ if(!ptr){ emraise( nullPtr, __VA_ARGS__ ); } }
# else  /* pure C --- use wrapper */
# define DBG_NULLPTR_CHECK( ptr, ... ) \
{ if(!ptr){ emraise( nullPtr, __VA_ARGS__ ); } }
# endif
# endif  /* NDEBUG */


/*! @} */

# endif  /* H_GOO_EXCEPTION_H */

