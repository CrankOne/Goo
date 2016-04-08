# include <cstring>
# include "utest.hpp"
# include "goo_dict/insertion_proxy.tcc"

/**@file pdict.cpp
 * @brief Parameters dictionary test.
 *
 * TODO
 * */

GOO_UT_BGN( PDICT, "Parameters dictionary routines" ) {
    // const char * const argv_[] = {};

    goo::dict::Configuration conf( "theApplication", "Testing parameter set." );

    conf.insertion_proxy()
        .p<bool>( '1', "parameter-one",  "First parameter" )
        .p<bool>( 'v', "Enables verbose output" )
        .p<bool>( 'q', "Be quiet", true )
        .p<bool>( "quet", "Be quiet", true )
        .p<bool>( "verbose", "Enables verbose output" )
        .p<bool>( 'V', "verbose2", "Enables verbose output" )
        .p<bool>( 'Q', "quet2", "Be quiet", true )
        //.p<bool>( 12, "one", "two", "three" )  // should be failed at linkage
        //.p<bool>( "one", "two", "three" )  // should be failed at linkage
        ;

    //conf.usage_text( os );

} GOO_UT_END( PDICT )

