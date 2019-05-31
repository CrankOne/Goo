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
    typedef std::unordered_map<std::string, dag::Node<TestingUnit>*> Registry;
private:
    /// Set to nullptr when cout is used.
    std::stringstream * _ss;
    /// The DAG is not a storage.
    static Registry * _modulesGraphPtr;
    //static std::unordered_map<std::string, TestingUnit *> * _modulesStoragePtr;  // XXX
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
        virtual ~TestingUnit();
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

/// Starts an "expect throw" block.
# define ut_Goo_EXPECT_THROW_BGN { bool wasThrown = false; try

# define _m_Goo_COMMON_WRONG_RAISED(excType, description)                       \
  catch( ... ) { emraise( uTestFailure, "Wrong exception type arised."          \
        " Expected \"%s\".", typeid(excType).name() );                          \
}                                                                               \
    if( !wasThrown ) {                                                          \
        emraise( uTestFailure, "Exception was not thrown while it was expected. " \
                description );                                                  \
    }                                                                           \
}

/// Ends an "expect throw" block.
# define ut_Goo_EXPECT_THROW_END( excType, description )                        \
  catch( excType ) { wasThrown = true; }                                        \
_m_Goo_COMMON_WRONG_RAISED( excType, description )

/// Ends an "expect throw" block, alternative for Goo's exceptions.
# define ut_Goo_EXPECT_THROW_CODE_END( expCode, description )                   \
  catch( goo::Exception & e ) {                                                 \
    if( goo::Exception:: expCode != e.code() ) {                                \
        emraise( uTestFailure, "Wrong Goo exception type arised. Got %d,"       \
                " expected %d.", e.code(), goo::Exception:: expCode );          \
    } wasThrown = true;                                                         \
} _m_Goo_COMMON_WRONG_RAISED( goo::Exception, description )



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

