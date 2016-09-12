# ifndef H_GOO_APPLICATION_H
# define H_GOO_APPLICATION_H

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

# include <sys/types.h>
# include <unistd.h>
# include <signal.h>
# include <string>
# include <map>
# include <unordered_map>
# include <list>
# include <utility>
# include <vector>
# include <typeinfo>
# include <cstdlib>
# include <ostream>
# include <cassert>

# include "goo_types.h"
# include "goo_exception.hpp"

namespace goo {

template<typename ConfigObjectT,
         typename LogStreamT> class App;

namespace aux {




/// Abstract application base class.
class iApp {
public:
    /// System signal handler callback type; only difference from std UNIX
    /// handler type is bool returning value indicating whether default
    /// action must be avoided. To avoid default behaviour after all hanlers
    /// were triggered, at least one of them should return true.
    typedef void(*SystemSignalHandler)(int, siginfo_t *, void*);

    enum HandlerResult : UByte {
        stopHandling = 0x1,
        omitDefaultAction = 0x2,
    };

    /// Returns flags composed with HandlerResult entries.
    typedef UByte(*SignalHandler)(int, siginfo_t *, void*);

    /// System signal code (only POSIX).
    enum SignalCode {
        _SIGHUP   = SIGHUP,   ///< Hangup
        _SIGINT   = SIGINT,   ///< Terminal interrupt signal.
        _SIGQUIT  = SIGQUIT,  ///< Quit from keyboard.
        _SIGILL   = SIGILL,   ///< Illegal instruction.
        _SIGFPE   = SIGFPE,   ///< Erroneous arithmetic operation.
        _SIGABRT  = SIGABRT,  ///< Abort signal from abort() (see $ man 3 abort).
        _SIGUSR1  = SIGUSR1,  ///< User-defined signal 1.
        _SIGSEGV  = SIGSEGV,  ///< Invalid memory reference.
        _SIGUSR2  = SIGUSR2,  ///< User-defined signal 2.
        _SIGPIPE  = SIGPIPE,  ///< Write on a pipe with no one to read it.
        _SIGALRM  = SIGALRM,  ///< Alarm clock.
        _SIGTERM  = SIGTERM,  ///< Termination signal.
        _SIGCHLD  = SIGCHLD,  ///< Child process terminated, stopped, or continued.
        _SIGCONT  = SIGCONT,  ///< Continue executing, if stopped.
        _SIGTSTP  = SIGTSTP,  ///< Terminal stop signal.
        _SIGTTIN  = SIGTTIN,  ///< Terminal input for background process.
        _SIGTTOU  = SIGTTOU,  ///< Terminal output for background process.

    };

    struct HandlerEntry {
        bool typeIsSystem;
        union {
            SystemSignalHandler _system;
            SignalHandler _custom;
        };
        std::string description;
    };

    # ifdef GDB_EXEC
    /// Attach gdb signal handler callback.
    static UByte attach_gdb(int, siginfo_t *, void*);
    # endif

    # ifdef GCORE_EXEC
    /// Core dumping signal handler callback.
    static UByte dump_core(int, siginfo_t *, void*);
    # endif

protected:
    /// Registered handlers. Should be invoked in order of addition.
    static std::map<SignalCode, std::list<HandlerEntry> > * _handlers;

    /// Stores documentation for environment variables.
    static std::unordered_map<std::string, std::string> * _documentedEnvVars;

    /// Private method that dispatches system signals to app.
    static void _signal_handler_dispatcher(int signum, siginfo_t *info, void * context);

    /// Private constructor that should only be used by App-subclass type.
    iApp() {}
    virtual ~iApp(){}

    /// Used by dispatcher routine instead of System V's ucontext_t.
    static iApp * _self;

    /// Configured application entry point pure-virtual function.
    virtual int _V_run() = 0;
public:
    /// Returns application instance.
    static iApp & self() { assert(_self); return *_self; }

    /// Returns true, if instance was created.
    static bool exists() { return !!_self; }

    /// C++ alias to standart UNIX hethostname() function.
    static std::string hostname();

    //
    // Work with signal handlers

    /// Adds new handler to handlers stack.
    static void add_handler(
        SignalCode,
        SignalHandler,
        const std::string,
        bool preservePrevious=true );

    /// Prints bound hadlers to stream.
    static void dump_signal_handlers( std::ostream & );

    /// Alias to standard UNIX getpid() function.
    static pid_t PID() { return getpid(); }

    //
    // Work with environment variables

    /// Goo's alias to acquire environment variable (std::getenv())
    static std::string envvar( const std::string &, const char * default_=nullptr );

    /// Parses environment variable as logical one.
    /// Receptive to "1", "true", "yes" or "0", "false", "no".
    /// Note, that in contrary to envvar() absence of
    /// the variable will cause returning false (not the noSuchKey
    /// exception).
    static bool envvar_as_logical( const std::string & );

    /// Adds envvar description that is printed in help message.
    static void add_environment_variable(
                const std::string & name,
                const std::string & description );

    /// Prints formatted message describing documented environment
    /// variables.
    static void dump_envvars( std::ostream & );

    template<typename ConfigObjectT,
             typename LogStreamT> friend class goo::App;
};

}  // namespace goo::aux


/**@brief Application object acquizition shortcut.
 *
 * Since abstract App singletone itself provides a self()-method
 * only for it's own type we need downcast it in order to get a 
 * concrete class instance. Dynamic casting is slow, so we should
 * cache it via special laconic template function. The best
 * practice is define alias to this function invokation in user's
 * code just after concrete application class declaration.
 */
template<typename ConcreteAppType>
ConcreteAppType & app() {
    try {
        static ConcreteAppType & casted = 
            dynamic_cast<ConcreteAppType&>(ConcreteAppType::self());
        return casted;
    } catch(std::bad_cast & e) {
        emraise(badCast, "Invalid application object type casting.");
    }
}



/**@brief Abstract application template.
 *
 * ConfigObjectT is a template parameter describing currently
 * used config object (e.g. boost::variables_map). This object
 * should be allocated and initialized at _V_configure_app()
 * pure-virtual method.
 *
 * LogStreamT is a template parameter describing currently used
 * logging stream object (e.g. std::stringstream or std::cout).
 * Concrete onject should be acquired by _V_acquire_stream()
 * pure-virtual method.
 */
template<typename ConfigObjectT,
         typename LogStreamT>
class App : public aux::iApp {
public:
    typedef App<ConfigObjectT, LogStreamT> SelfAbstractType;
private:
    /// Stream for standard logging.
    LogStreamT  * _lStr;
    /// Config object.
    ConfigObjectT * _cObj;
protected:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual ConfigObjectT * _V_construct_config_object( int argc, char * const argv[] ) const = 0;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const ConfigObjectT * ) = 0;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual LogStreamT * _V_acquire_stream() = 0;
protected:
    App() : _lStr(nullptr), _cObj(nullptr) {}
    virtual ~App() {}
public:

    // general application management

    /// Creates application instance. Must be invoked just after entry point.
    static SelfAbstractType * init(int argc_, char * argv_[], App<ConfigObjectT, LogStreamT> * app) {
        _self = app;
        app->_V_configure_application(
            app->_cObj = app->_V_construct_config_object(argc_, argv_) );
        app->_lStr = app->_V_acquire_stream();
        return app;
    }

    /// Configured application entry point.
    static int run() { int rc = _self->_V_run();
                       delete _self; _self = nullptr;
                       return rc; }

    // methods

    /// Returns reference on common loging stream object.
    inline LogStreamT    & ls() { assert(_lStr); return *_lStr; }
    /// Returns whether or not log instance object was set.
    inline bool ls_is_set() const { return _lStr; }
    /// Returns reference on common config object.
    inline ConfigObjectT & co() { assert(_cObj); return *_cObj; }
    /// Returns reference on common config object (const version).
    inline const ConfigObjectT & co() const { assert(_cObj); return *_cObj; }
    /// Returns whether or not config instance object was set.
    inline bool co_is_set() const { return _cObj; }
};

}  // namespace goo

# endif  // H_GOO_APPLICATION_H

