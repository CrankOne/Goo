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

# include <cerrno>
# include <cstring>
# include <regex>
# include "goo_utility.h"
# include "goo_app.hpp"

namespace goo {

//
// iApp
//

/**@namespace aux
 *
 * Sys namespace constains Goo auxillary classes. One usually
 * has no need to deal with these objects.
 */ namespace aux {

/**@class iApp
 * One probably should never inherit this class directly.
 *
 * Provides some non-template UNIX-specific system routines
 * like signal handling or environment variables parsing.
 *
 * One can insert a number of signal handlers.
 * When signal received, handling functions will be
 * invoked in reverse order of insertion (the oldest is last).
 *
 * For example following signal can be handled (explainations taken
 * from http://www.yolinux.com/TUTORIALS/C++Signals.html):
 *  + SIGHUP (1)    -- Hangup (POSIX) Report that user's terminal is
 *                  disconnected. Signal used to report the termination
 *                  of the controlling process.
 *  + SIGINT (2)    -- Interrupt (ANSI) Program interrupt. (ctrl-c).
 *  + SIGFPE (8)    -- Floating-Point arithmetic Exception (ANSI). This
 *                  includes division by zero and overflow. The
 *                  IEEE Standard for Binary Floating-Point Arithmetic
 *                  (ANSI/IEEE Std 754-1985) defines various
 *                  floating-point exceptions.
 *  + SIGUSR1 (10)  -- User-defined signal 1
 *  + SIGSEGV (11)  -- Segmentation Violation (ANSI). Occurs when a program
 *                  tries to read or write outside the memory that is
 *                  allocated for it by the operating system, dereferencing
 *                  a bad or NULL pointer. Indicates an invalid access to
 *                  valid memory.
 *  + SIGUSR2 (12)  -- User-defined signal 2
 *  + SIGPIPE (13)  -- Broken pipe (POSIX) Special error condition.
 *                  E.g. attempt to write to a socket which is not
 *                  connected.
 *  + SIGALRM (14) -- Alarm clock (POSIX) Indicates expiration of a
 *                  timer. Used by the alarm() function.
 *  + SIGTERM (15)  -- Termination (ANSI) This signal can be blocked,
 *                  handled, and ignored. Generated by "kill" command.
 *  + SIGCHLD (17)  -- Child status has changed (POSIX) Signal sent to
 *                  parent process whenever one of its child processes
 *                  terminates or stops.
 *  + SIGCONT (18)  -- Continue (POSIX) Signal sent to process to make
 *                  it continue.
 *  + SIGTSTP (20)  -- Keyboard stop (POSIX) Interactive stop signal.
 *                  This signal can be handled and ignored. (ctrl-z)
 *
 * Description of other supported signals can be obtained by `$ man 7 signal`.
 */

iApp * iApp::_self = nullptr;

DECLTYPE( iApp::_documentedEnvVars ) iApp::_documentedEnvVars = nullptr;
DECLTYPE( iApp::_handlers ) iApp::_handlers = nullptr;

iApp &
iApp::self() {
    if( !_self ) {
        emraise( badState, "None application iApp instantiated while its "
                 "instance required." );
    }
    return *_self;
}

/** Raises 'nwGeneric' exception on failure.
 */
std::string
iApp::hostname() {
    char bf[128];
    if( -1 == gethostname( bf, 128 ) ) {
        emraise( nwGeneric, "Failed to get hostname: (%d) %s",
            errno, strerror(errno) );
    }
    return bf;
}

//
// Signal handlers

/** The preservePrevious parameter has only sense when currently adding
 * handler is first. Whein preservePrevious=true, the previously assigned
 * handler will be preserved and put to the end of handlers list for
 * certain signal. It is useful wheen one need to keep signals set by
 * some third-party frameworks (like Geant4, CERN ROOT, etc) but want
 * to perform some custom operations first.
 *
 * @todo Elaborate this facility support extended functions provided by
 * `sa_flags` field of `struct sigaction` in `<unistd.h>`.
 *
 * @param code UNIX signal ID.
 * @param fun callback instance
 * @param preservePrevious keep non-default handler previously set (if has).
 */
void
iApp::add_handler( SignalCode code,
                   SignalHandler f,
                   const std::string description,
                   bool preservePrevious ) {
    if( !_handlers ) {
        _handlers = new std::remove_pointer<DECLTYPE(_handlers)>::type ();
    }
    // try to find and append appropriate handlers list
    auto handlersStackIt = _handlers->find( code );
    if( handlersStackIt != _handlers->end() ) {
        // Append list:
        handlersStackIt->second.push_back(
                HandlerEntry{
                        false,
                        {._custom = f},
                        description
                    }
            );
        # ifndef NDEBUG
        if( preservePrevious && 1 != handlersStackIt->second.size() ) {
            dprintf( "Default system "
                     "handler suppression has no effect when adding "
                     "handler is not first in stack.\n" );
        }
        # endif
        return;  // appending done, exit
    }

    // signal unbound -- bind own dispatcher to treat the signal
    struct sigaction act;
    struct sigaction oldAct;
    bzero( &act, sizeof(act) );
    bzero( &act, sizeof(oldAct) );
    act.sa_sigaction = &(iApp::_signal_handler_dispatcher);
    act.sa_flags = SA_SIGINFO
                 //| SA_ONSTACK
                 ;
    if( sigaction(code, &act, &oldAct) < 0 ) {
        emraise(thirdParty, "sigaction() returned an error: %s.\n",
                            strerror(errno) );
    }
    // Signal bound to dispatcher, now append the handlers stack, if
    // something was previously set.
    std::remove_pointer<DECLTYPE(_handlers)>::type::mapped_type lst;
    if( preservePrevious ) {
        if( oldAct.sa_handler != SIG_DFL ) {
            lst.push_front( HandlerEntry{
                        true,
                        {._system = oldAct.sa_sigaction},
                        "Handler set by third-party code."
                    } );
        } else {
            // We use here a warning message because it may be crucial sometimes
            // to know that third-party software hadn't yet set their handler(s).
            // This behaviour usually must be under strict control, so producing a
            // warning message should be a good hint.
            wprintf( "No third-party handler overrides a signal %s(%d) at moment.\n",
                     strsignal(code), code );
        }
    }
    lst.push_back( HandlerEntry{
                        false,
                        {._custom = f},
                        description
                    } );
    _handlers->emplace(code, lst);
};

/** Uses STL's stream instance to dump registered handlers
 * line-by-line in format:
 *  Signal: <signal ID>
 *      <callback pointer address> <user's callback description>
 *      ...
 * or prints "<default handlers>" if there is no one registered.
 *
 * @param os STL's ostream to print in.
 */ void
iApp::dump_signal_handlers( std::ostream & os ) {
    if( !_handlers || _handlers->empty() ) {
        os << "<default handlers>" << std::endl;
        return;
    }
    for( auto it = _handlers->cbegin(); it != _handlers->cend(); ++it ) {
        os << "Signal: " << strsignal(it->first) << " (" << it->first << ")" << std::endl;
        for(auto iit = it->second.cbegin(); iit != it->second.cend(); ++iit){
            os << "  " << (void *) iit->_system << " " << iit->description << std::endl;
        }
    }
}

void
iApp::_signal_handler_dispatcher( int signum,
                                  siginfo_t *info,
                                  void * context ) {
    assert( _handlers );
    auto entry = iApp::self()._handlers->find( (SignalCode) signum );
    uint8_t interruptFlags = 0x0;
    for( auto it  = entry->second.crbegin();
              it != entry->second.crend(); ++it){
        if( it->typeIsSystem ) {
            (it->_system)( signum, info, context );
        } else {
            interruptFlags |= (it->_custom)( signum, info, context );
            if( interruptFlags & stopHandling ) {
                break;
            }
        }
    }
    if( interruptFlags & omitDefaultAction ) {
        return;
    }

    signal( signum, SIG_DFL );
}

# ifdef GOO_GDB_EXEC
UByte
iApp::attach_gdb(int, siginfo_t *, void*) {
    char buf[128]; int rc;
    snprintf( buf, sizeof(buf), "%s -p %d", GOO_GDB_EXEC, getpid() );
    // TODO: this system call should be delegated to safe wrapper.
    // May some combination of fork() and then excl() do
    // the trick?
    fprintf(stdout, "Invokation of: " ESC_CLRBOLD "$ %s" ESC_CLRCLEAR "\n", buf);
    rc = system(buf);
    (void)(rc);
    return 0x0;
}
# endif

# ifdef GOO_GCORE_EXEC
UByte
iApp::dump_core(int, siginfo_t *, void*) {
    char buf[128]; int rc;
    snprintf( buf, sizeof(buf), "%s %d", GOO_GCORE_EXEC, getpid() );
    // TODO: this system call should be delegated to safe wrapper.
    // May some combination of fork() and then excl() do
    // the trick?
    fprintf(stdout, "Invokation of: " ESC_CLRBOLD "$ %s" ESC_CLRCLEAR "\n", buf);
    rc = system(buf);
    (void)(rc);
    return 0x0;
}
# endif

//
// Environment variables

void
iApp::add_environment_variable(
                const std::string & name,
                const std::string & description ) {
    if( !_documentedEnvVars ) {
        _documentedEnvVars = new std::remove_pointer<DECLTYPE(_documentedEnvVars)>::type ();
    }
    auto insertionResult = _documentedEnvVars->emplace( name, description );
    if( !insertionResult.second ) {
        wprintf( "Environment variable \"%s\" description is already known "
                 "and wasn't overriden.\n", name.c_str() );
    }
}

/**Second parameter specifies the value to be returned if
 * there is no such environment variable or it is empty. If
 * default value is nullptr, raises noSuchKey.
 *
 * @param nm        name of environment variable (e.g. PATH)
 * @param default_  string value to be returned if no such envvar.
 */ std::string
iApp::envvar( const std::string & nm, const char * default_ ) {
    # ifndef NDEBUG
    if( !_documentedEnvVars
      || _documentedEnvVars->end() == _documentedEnvVars->find(nm) ) {
        wprintf( "(dev) Environment variable %s is not documented.\n",
                 nm.c_str() );
    }
    # endif
    char * var = ::std::getenv(nm.c_str());
    if(!var || '\0' == *var ) {
        if(!default_) {
            emraise(noSuchKey, "Environment variable %s is not defined.", nm.c_str());
        } else {
            return default_;
        }
    }
    return var;
}


bool
iApp::envvar_as_logical( const std::string & envVarName ) {
    # ifndef NDEBUG
    if( !_documentedEnvVars
      || _documentedEnvVars->end() == _documentedEnvVars->find(envVarName) ) {
        wprintf( "(dev) Environment variable %s is not documented.\n",
                 envVarName.c_str() );
    }
    # endif
    char * var = ::std::getenv(envVarName.c_str());
    if( !var ) {
        return false;
    }
    // use regex to parse
    static const std::regex trueRx(  "(enable|yes|true|on|1)",   std::regex::ECMAScript | std::regex::icase ),
                            falseRx( "(disable|no|false|off|0)", std::regex::ECMAScript | std::regex::icase )
                            ;
    std::cmatch cm;
    if( std::regex_match( var, cm, trueRx ) ) {
        return true;
    } else if( std::regex_match( var, cm, falseRx ) ) {
        return false;
    } else {
        wprintf( "Couldn't interpret environment variable value %s:\"%s\" "
                 "as a logical literal. \"False\" value returned.\n",
                 envVarName.c_str(), var );
        return false;
    }
}

/** Uses STL's stream instance to dump registered environment vars
 * line-by-line in form:
 *  Envvar: <signal ID> #<execNum>
 *      <description>
 *      ...
 * or prints "<No envvars look-up>" if there is no one known.
 *
 * @param os STL's ostream to print in.
 */ void
iApp::dump_envvars( std::ostream & os ) {
    if( !_documentedEnvVars || _documentedEnvVars->empty() ) {
        os << "<No envvars look-up>" << std::endl;
        return;
    }
    for( auto p : *_documentedEnvVars ) {
        os << p.first << " --- " << p.second << std::endl;
    }
}

}  // namespace goo::aux


//
// App
//

};  // namespace goo

