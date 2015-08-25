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
    static LabeledDAG<TestingUnit> _modulesGraph;
public:
    class TestingUnit {
    private:
        virtual void _V_run( std::ostream & ) = 0;
        std::ostream * _outStream;
    public:
        TestingUnit() : _outStream(&(std::cout)) {}
        virtual ~TestingUnit(){}
        void run( ) { _V_run( *_outStream ); }
        std::ostream & outs() { return *_outStream; }
        void outs( std::ostream & os ) { _outStream = &os; }
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
        printHelp           = 0,
        printBuildConfig    = 1,
        listUnits           = 2,
        dumpDOT             = 3,
        runAll              = 4,
        runChoosen          = 5,
    } operation;

    /// Supplementary options.
    bool silent,
         keepGoing,
         printUnitsLogs,
         ignoreDeps;

    /// Vector of unit names desired to run.
    std::vector<std::string> names;

    /// Prepared unit sequence to run.
    LabeledDAG<UTApp::TestingUnit>::Order units;
};

}  // namespace ut
}  // namespace goo

# define GOO_UT_BGN( name )                                         \
static goo::ut::UTApp::TestingUnit * __ptr_UT_ ## name = nullptr;   \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));       \
namespace goo { namespace ut {                                      \
class UT_ ## name : public goo::ut::UTApp::TestingUnit {            \
protected:                                                          \
virtual void _V_run(std::ostream & out) override {

# define _ASSERT( expr, ... )                                       \
if( !(expr) ) { emraise(uTestFailure, __VA_ARGS__ ) }

# define GOO_UT_END( name, ... ) } };                               \
void __ut_ctr_ ## name(){                                           \
__ptr_UT_ ## name = new UT_ ## name();                              \
goo::ut::UTApp::register_unit( #name, __ptr_UT_ ## name ); }        \
void __ut_dtr_ ## name() __attribute__(( destructor(156) ));        \
void __ut_dtr_ ## name(){ if(__ptr_UT_ ## name) {                   \
    delete __ptr_UT_ ## name;} } } }

# if 0
namespace gooUT {

class Unit {
protected:
    std::string         _name;
    std::stringstream   _ss;
    static std::map<std::string, Unit *> * _units;

    virtual void _run( std::stringstream & ) = 0;
public:
    explicit Unit( const std::string & name ) : _name(name) {
        if( !_units ) {
            Unit::_units = new std::map<std::string, Unit *>();
        }
        (*_units)[name] = this;
    }
    virtual ~Unit() {
        if(_units) {
            _units->erase( _name );
        }
        if( _units->empty() ) {
            delete _units;
            _units = nullptr;
        }
    }
    inline std::string name() const { return _name; }
    inline std::string get_report() const { return _ss.str(); }


    virtual void run();

    static bool run_tests( int argc, char * argv[] );
    static void enlist_modules( std::ostream & );

    friend void __init_units_dictionary();
    friend void __free_units_dictionary();
};

}  // namespace HphST

# define GOO_UT_BGN( name )                                     \
static gooUT::Unit * __ptr_UT_ ## name = nullptr;               \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));   \
namespace gooUT { \
class UT_ ## name : public Unit {             \
public: UT_ ## name() : Unit( # name ) {}; protected:           \
virtual void _run(std::stringstream & out) { using namespace goo;

# define _ASSERT( expr, ... ) \
if( !(expr) ) { hraise(uTestFailure, __VA_ARGS__ ) }

# define GOO_UT_END( name ) } }; \
} void __ut_ctr_ ## name(){ __ptr_UT_ ## name = new gooUT::UT_ ## name(); } \
void __ut_dtr_ ## name() __attribute__(( destructor(156) ));   \
void __ut_dtr_ ## name(){ if(__ptr_UT_ ## name) {delete __ptr_UT_ ## name;} }
# else
// ...
# endif

# endif  // H_HPHST_UTEST_H

