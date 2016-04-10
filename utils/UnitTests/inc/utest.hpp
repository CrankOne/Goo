# ifndef H_GOO_UTEST_H
# define H_GOO_UTEST_H

# include <string>
# include <map>
# include <iostream>
# include <sstream>

# include "goo_app.hpp"
# include "goo_exception.hpp"
# include "goo_tsort.tcc"

namespace goo {
namespace ut {

struct Config;

class UTApp : public goo::App<Config, std::ostream> {
public:
    class TestingUnit;
    typedef goo::App<Config, std::ostream> Parent;
private:
    /// Set to nullptr when cout is used.
    std::stringstream * _ss;
    /// The DAG is not a storage.
    static LabeledDAG<TestingUnit>                        * _modulesGraphPtr;
    static std::unordered_map<std::string, TestingUnit *> * _modulesStoragePtr;
public:
    class TestingUnit {
    private:
        virtual void _V_run( std::ostream & ) = 0;
        bool _outStreamOwn;
        std::ostream * _outStream;
        std::string _verboseName;   ///< Unit description comment.
        std::unordered_set<std::string> _depNames;
        int _ranResult;  ///< >0 if not run, ==0 if ok, <0 if failed.
    public:
        /// Ctr -- gets a verbose (displayed) name of module.
        TestingUnit( const std::string & verboseName ) :
                _outStreamOwn( false ),
                _outStream( &(std::cout) ),
                _verboseName( verboseName ),
                _ranResult(1) {}
        ~TestingUnit();
        /// Runs module. Can throw exceptions.
        void run( bool dryRun=false ) noexcept;
        void run_unsafe( bool dryRun=false );
        void make_own_outstream();
        std::ostream & outs() { return *_outStream; }
        const std::ostream & outs() const { return *_outStream; }
        void outs( std::ostream & os ) { _outStream = &os; }
        const std::string & verbose_name() const { return _verboseName; }
        void set_dependencies( const char [][48], uint8_t depLength );
        const std::unordered_set<std::string> & dep_names() const { return _depNames; }
        int ran_result() const noexcept { return _ranResult; }
    };
private:
    static std::unordered_set<TestingUnit*> _modules;
    std::string _appName;
protected:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual Config * _V_construct_config_object( int argc, char * const argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const Config * ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual std::ostream * _V_acquire_stream() override;
    /// Run configured application.
    virtual int _V_run() override;
    /// Sets up all dependencies enlisted in units descriptions.
    void _incorporate_dependencies();
    /// Run module wrapper routine. Result < 0 indicates an error.
    int _run_unit( TestingUnit *, std::ostream &, bool noRun=false );
public:
    UTApp( const std::string & appname );
    ~UTApp();

    static void register_unit( const std::string & label,
                               TestingUnit * );
    static void list_modules( std::ostream & );
};

# define GOO_UT_BGN( name, verbName )                                           \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));                   \
namespace goo { namespace ut {                                                  \
class UT_ ## name : public goo::ut::UTApp::TestingUnit {                        \
public: UT_ ## name() : goo::ut::UTApp::TestingUnit( verbName ) {};             \
protected: virtual void _V_run(std::ostream & os) override { using namespace goo;

# define _ASSERT( expr, ... )                                                   \
if( !(expr) ) { emraise( uTestFailure, __VA_ARGS__ ); }

# define GOO_UT_END( name, ... ) } };                                           \
} } void __ut_ctr_ ## name(){                                                   \
    const char depsString[][48] = { __VA_ARGS__ };                              \
    goo::ut::UTApp::TestingUnit * utInst = new goo::ut::UT_ ## name();          \
    goo::ut::UTApp::register_unit( # name, utInst );                            \
    utInst->set_dependencies( depsString, sizeof(depsString)/48 );              \
}

}  // namespace ut
}  // namespace goo

# endif  // H_GOO_UTEST_H

