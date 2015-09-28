# ifndef H_HPHST_UTEST_H
# define H_HPHST_UTEST_H

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
    std::stringstream * _ss;
    /// The DAG is not a storage.
    static LabeledDAG<TestingUnit>                        * _modulesGraphPtr;
    static std::unordered_map<std::string, TestingUnit *> * _modulesStoragePtr;
public:
    class TestingUnit {
    private:
        virtual void _V_run( std::ostream & ) = 0;
        std::ostream * _outStream;
        std::string _name,          ///< Short name used to identify unit.
                    _verboseName;   ///< Unit description comment.
    public:
        TestingUnit( const std::string & name, const std::string & verboseName ) :
                _outStream( &(std::cout) ),
                _name( name ),
                _verboseName( verboseName ) {}
        void run( ) { _V_run( *_outStream ); }
        std::ostream & outs() { return *_outStream; }
        void outs( std::ostream & os ) { _outStream = &os; }
        const std::string & verbose_name() const { return _verboseName; }
        void set_dependencies( const char [][48], uint8_t depLength );
    };
private:
    static std::unordered_set<TestingUnit*> _modules;
protected:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual Config * _V_construct_config_object( int argc, char * argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const Config * ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual std::ostream * _V_acquire_stream() override;
    /// Run configured application.
    virtual int _V_run() override;
public:
    UTApp();
    ~UTApp();

    static void register_unit( const std::string & label,
                               TestingUnit * );
    static void list_modules( std::ostream & );
};

/// Unit test application config object.
struct Config {
    /// Available functions of application. For descriptions, see _V_construct_config_object().
    enum Operations {
        unassigned          = 0,
        printHelp           = 1,
        printBuildConfig    = 2,
        listUnits           = 3,
        dumpDOT             = 4,
        runAll              = 5,
        runChoosen          = 6,
    } operation;

    /// Supplementary options.
    bool quiet,
         keepGoing,
         printUnitsLogs,
         ignoreDeps;

    /// Vector of unit names desired to run.
    std::vector<std::string> names;

    /// Prepared unit sequence to run.
    mutable LabeledDAG<UTApp::TestingUnit>::Order units;
};

# define GOO_UT_BGN( name, verbName )                                           \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));                   \
namespace goo { namespace ut {                                                  \
class UT_ ## name : public goo::ut::UTApp::TestingUnit {                        \
public: UT_ ## name() : goo::ut::UTApp::TestingUnit( # name, verbName ) {};     \
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

# endif  // H_HPHST_UTEST_H

