# include <cstring>
# include <wordexp.h>
# include "utest.hpp"
# include "goo_dict/insertion_proxy.tcc"

/**@file pdict.cpp
 * @brief Parameters dictionary test.
 *
 * TODO
 * */

GOO_UT_BGN( PDICT, "Parameters dictionary routines" ) {
    // os << "**** **** ****" << std::endl;
    // const char * const argv_[] = {};

    {
        os << "Basic tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication", "Testing parameter set." );

        conf.insertion_proxy()
            .p<bool>( '1', "parameter-one",  "First parameter" )
            .p<bool>( 'v', "Enables verbose output" )
            .p<bool>( 'q', "Be quiet", true )
            .p<bool>( "quiet", "Be quiet", true )
            .p<bool>( "verbose", "Enables verbose output" )
            .p<bool>( 'V', "verbose2", "Enables verbose output" )
            .p<bool>( 'Q', "quet2", "Be quiet", true )
            //.p<bool>( 12, "one", "two", "three" )  // should be failed at linkage
            //.p<bool>( "one", "two", "three" )  // should be failed at linkage
            ;
        // Check default is set correctly:
        const char ex1[] = "./foo -1vqfalse --quiet=true --verbose -V --quet2 false";
        char ** argv;
        int argc = goo::dict::Configuration::tokenize_string( ex1, argv );
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << std::endl;
        }

        conf.extract( argc, argv, &os );

        goo::dict::Configuration::free_tokens( argc, argv );
        // TODO : check options are really set to expected values
        os << "} Basic tests done." << std::endl;
    }
    // TODO : expected for pasing errors check
    # if 1
    {
        os << "List parameters tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication", "Testing parameter set." );

        conf.insertion_proxy()
            .p<bool>( '1', "parameter-one",  "First parameter" )
            .list<bool>( 'b', "binary", "options array", {true, true, false, false} )
            .list<bool>( 'B', "options array", {false, true} )
            .list<bool>( "binary2", "options array" )
            .list<bool>( 'a', "options array" )
            .p<bool>( 'v', "Enables verbose output" )
            ;
        // Check default is set correctly:
        const char ex1[] = "./foo -1v -b true -b Off -b On --binary2 OFF --binary ON";
        char ** argv;
        int argc = goo::dict::Configuration::tokenize_string( ex1, argv );
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << std::endl;
        }

        conf.extract( argc, argv, &os );

        goo::dict::Configuration::free_tokens( argc, argv );
        os << "} List parameters done." << std::endl;
    }
    # endif

    //conf.usage_text( os );

} GOO_UT_END( PDICT )

