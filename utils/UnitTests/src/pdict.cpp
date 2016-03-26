# include <cstring>
# include "utest.hpp"
# include "goo_dict/insertion_proxy.tcc"

/**@file pdict.cpp
 * @brief Parameters dictionary test.
 *
 * TODO
 * */

GOO_UT_BGN( PDICT, "Parameters dictionary routines" ) {
    # if 0
    const char * const argv_[] = {
        "parameter-one",    "true",
        "parameter-two",    "false"
    };
    # endif

    goo::dict::Configuration conf( "theApplication", "Testing parameter set." );

    conf.insertion_proxy()
        .p<bool>( "parameter-one",  "Parameter `one'." )
        ;

    conf.usage_text( os );

} GOO_UT_END( PDICT )

