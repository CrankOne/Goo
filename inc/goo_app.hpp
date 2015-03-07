# ifndef H_GOO_APPLICATION_H
# define H_GOO_APPLICATION_H

# include <sys/types.h>
# include <unistd.h>
# include <signal.h>
# include <string>
# include <map>
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
    /// System signal handler callback type.
    typedef void(*SignalHandler)(int, siginfo_t *, void*);

    /// System signal code.
    enum SignalCode {
        _SIGHUP   = 1,          _SIGINT   = 2,          _SIGFPE   = 8,
        _SIGUSR1  = 10,         _SIGUSR2  = 12,         _SIGPIPE  = 13,
        _SIGALARM = 14,         _SIGTERM  = 15,         _SIGCHLD  = 17,
        _SIGCONT  = 18,         _SIGSTP   = 20,
    };
protected:
    /// Registered handlers. Should be invoked in order of addition.
    std::map<SignalCode,
                std::vector<
                    std::pair<SignalHandler, std::string> > > _handlers;

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

    /// Adds new handler to handlers stack.
    void add_handler( SignalCode, SignalHandler, const std::string, bool suppressDefault=false );

    /// Prints bound hadlers to stream.
    void dump_handlers( std::ostream & ) const;

    /// Alias to standard UNIX getpid() function.
    inline pid_t PID() const { return getpid(); }

    /// C++ alias to standart UNIX hethostname() function.
    std::string hostname() const;

    /// Goo's alias to acquire environmental variable (std::getenv())
    std::string envvar( const std::string & ) const;

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
    /// Stream for logging.
    LogStreamT  * _lStr;
    /// Config object.
    ConfigObjectT * _cObj;

protected:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual ConfigObjectT * _V_construct_config_object( int argc, char * argv[] ) const = 0;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const ConfigObjectT * ) = 0;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual LogStreamT * _V_acquire_stream() = 0;
protected:
    int argc;
    char ** argv;
    App() : _lStr(nullptr), _cObj(nullptr) {}
    virtual ~App() {}
public:

    // general application management

    /// Creates application instance. Must be invoked just after entry point.
    static SelfAbstractType * init(int argc_, char * argv_[], App<ConfigObjectT, LogStreamT> * app) {
        _self = app;
        app->argc = argc_; app->argv = argv_;
        app->_V_configure_application(
            app->_cObj = app->_V_construct_config_object(argc_, argv_) );
        app->_lStr = app->_V_acquire_stream();
        return app;
    }

    /// Configured application entry point.
    static int run() { int rc = _self->_V_run();
                       delete _self;
                       return rc; }

    // methods

    /// Returns reference on common loging stream object.
    inline LogStreamT    & ls() { assert(_lStr); return *_lStr; }
    /// Returns reference on common config object.
    inline ConfigObjectT * co() { assert(_cObj); return  _cObj; }
    /// Returns reference on common config object (const version).
    inline const ConfigObjectT * co() const { assert(_cObj); return *_cObj; }
};

}  // namespace goo

# endif  // H_GOO_APPLICATION_H

