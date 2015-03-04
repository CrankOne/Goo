# ifndef H_HPHST_UTEST_H
# define H_HPHST_UTEST_H

# include <string>
# include <map>
# include <sstream>

# include "goo_app.hpp"
# include "goo_exception.hpp"


namespace goo {
namespace ut {

class UTApp : public goo::App<void, std::ostream> {
public:
    class TestingUnit {
    private:
        std::string         _name;
        std::ostream & _outStream,
                     & _errStream;
        virtual void _run() = 0;
    public:
        void run( ) { _run(); }
        std::string get_out_log() { return _outStream.str(); }
        std::string get_err_log() { return _errStream.str(); }
    };
private:
    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual ConfigObjectT * _V_construct_config_object( int argc, char * argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const ConfigObjectT & ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual LogStreamT * _V_acquire_stream() override;
};

}  // namespace ut
}  // namespace goo

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

# define HPH_UT_BGN( name )                                     \
static gooUT::Unit * __ptr_UT_ ## name = nullptr;               \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));   \
namespace gooUT { \
class UT_ ## name : public Unit {             \
public: UT_ ## name() : Unit( # name ) {}; protected:           \
virtual void _run(std::stringstream & out) { using namespace goo;

# define _ASSERT( expr, ... ) \
if( !(expr) ) { hraise(uTestFailure, __VA_ARGS__ ) }

# define HPH_UT_END( name ) } }; \
} void __ut_ctr_ ## name(){ __ptr_UT_ ## name = new gooUT::UT_ ## name(); } \
void __ut_dtr_ ## name() __attribute__(( destructor(156) ));   \
void __ut_dtr_ ## name(){ if(__ptr_UT_ ## name) {delete __ptr_UT_ ## name;} }
# endif  // 0


# endif  // H_HPHST_UTEST_H

