# ifndef H_HPHST_UTEST_H
# define H_HPHST_UTEST_H

# include <string>
# include <map>
# include <sstream>
# include "hph_exception.hpp"

namespace hphUT {

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
            _units = null;
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
static hphUT::Unit * __ptr_UT_ ## name = null;                  \
void __ut_ctr_ ## name() __attribute__(( constructor(156) ));   \
namespace hphUT { \
class UT_ ## name : public Unit {             \
public: UT_ ## name() : Unit( # name ) {}; protected:           \
virtual void _run(std::stringstream & out) { using namespace hph;

# define _ASSERT( expr, ... ) \
if( !(expr) ) { hraise(uTestFailure, __VA_ARGS__ ) }

# define HPH_UT_END( name ) } }; \
} void __ut_ctr_ ## name(){ __ptr_UT_ ## name = new hphUT::UT_ ## name(); } \
void __ut_dtr_ ## name() __attribute__(( destructor(156) ));   \
void __ut_dtr_ ## name(){ if(__ptr_UT_ ## name) {delete __ptr_UT_ ## name;} }

# endif  // H_HPHST_UTEST_H

