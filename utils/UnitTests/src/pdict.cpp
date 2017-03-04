# include <cstring>
# include <wordexp.h>
# include "utest.hpp"
# include "goo_dict/insertion_proxy.tcc"
# include "goo_dict/parameters/integral.tcc"

/**@file pdict.cpp
 * @brief Parameters dictionary test.
 *
 * TODO
 * */

GOO_UT_BGN( PDICT, "Parameters dictionary routines" ) {
    # if 1
    {
        os << "Path splitting regex : {" << std::endl;
        int rc;
        char    one[] = "one.three-4.five.six-seven",
                two[] = "v",
              three[] = "12one-7.1",
            * path,
            * current ;
        {
            os << "Splitting \"" << one << "\" (common case):" << std::endl;
            _ASSERT( 1 == (rc = goo::dict::Dictionary::pull_opt_path_token( path = one, current )),
                        "Wrong path subtoken #1: %d.", rc );
            _ASSERT( !strcmp("one", current) && !strcmp("three-4.five.six-seven", path),
                     "#1 was splitted to \"%s\" and \"%s\"",
                     current, path );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        {
            os << "Splitting \"" << two << "\" (single-char option remained):" << std::endl;
            _ASSERT( 0 == (rc = goo::dict::Dictionary::pull_opt_path_token( path = two, current )),
                        "Wrong path subtoken #2: %d.", rc  );
            _ASSERT( !strcmp("v", current) && 0 == strlen(path) && 1 == (path - current),
                     "#2 was splitted to \"%s\" and string of length %d; path - current = %d",
                     current, (int) strlen(path), (int) (path - current) );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        {
            os << "Splitting \"" << three << "\" (dictionary short name):" << std::endl;
            _ASSERT( 1 == (rc = goo::dict::Dictionary::pull_opt_path_token( path = three, current )),
                        "Wrong path subtoken #3: %d.", rc  );
            _ASSERT( !strcmp("12one-7", current) && !strcmp("1", path) && 1 != (path - current),
                     "#3 was splitted to \"%s\" and string of length %d",
                     current, (int) strlen(path) );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        os << "} Path splitting regex done." << std::endl;
    }
    # endif
    # if 1
    {
        os << "Basic tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication1", "Testing parameter set #1." );

        conf.insertion_proxy()
            .flag(    '1', "parameter-one",  "First parameter" )
            .flag(    'v', "Enables verbose output" )
            .p<bool>( 'q', "Be quiet", true )
            .p<bool>( "quiet", "Be quiet", true )
            .p<bool>( "verbose", "Enables verbosity" )
            .p<short>(  "verbosity", "Sets verbosity level" )
            .flag(    'V', "verbose2", "Enables verbose output" )
            .p<bool>( 'Q', "quiet2", "Be quiet", true )
            //.p<bool>( 12, "one", "two", "three" )  // should cause failure on linkage
            //.p<bool>( "one", "two", "three" )  // should cause failure on linkage
            ;

        const char ex1[] = "./foo -1vqfalse --quiet=true -V --quiet2 false --verbosity 23";
        char ** argv;
        os << "For given source string: " << ex1 << ":" << std::endl;
        int argc = goo::dict::Configuration::tokenize_string( ex1, argv );
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << "|";
        }
        os << std::endl;

        conf.extract( argc, argv, false, &os );

        goo::dict::Configuration::free_tokens( argc, argv );
        // check options are really set to expected values
        _ASSERT(  conf["1"].as<bool>(),         "Option -1 set wrong." );
        _ASSERT(  conf["v"].as<bool>(),         "Option -v set wrong." );
        _ASSERT( !conf["q"].as<bool>(),         "Option -q set wrong." );
        _ASSERT(  conf["quiet"].as<bool>(),     "Option --quiet set wrong." );
        _ASSERT(  23 == conf["verbosity"].as<short>(),  "Option --verbosity set wrong." );

        try {
            conf["verbose"].as<bool>();
        } catch( goo::Exception & e ) {
            if( goo::Exception::uninitialized != e.code() ) { throw; }
            // ok, this exception expected.
        }

        _ASSERT(  conf["verbose2"].as<bool>(),
                "Option --verbose2 set to wrong value." );
        _ASSERT( !conf["quiet2"].as<bool>(),
                "Option --quiet2 set wrong to wrong value." );
        // ...
        os << "} Basic tests done." << std::endl;
    }
    # endif
    // TODO : expected for pasing errors check
    # if 1
    {
        os << "List parameters tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication2", "Testing parameter set #2." );

        conf.insertion_proxy()
            .flag( '1', "parameter-one",  "First parameter" )
            .list<bool>( 'b', "binary", "options array #1",
                                                {true, true, false, false} )
            .list<bool>( 'B', "options array #2", {false, false, false, false} )
            .list<bool>( "binary2", "options array #3" )
            .list<bool>( 'a', "options array #4", {true, false, true} )
            .flag( 'v', "Enables verbose output" )
            //.list<short>( 'x', "List of short inegers", { 112, 53, 1024 } )
            ;

        const char ex1[] = "./foo -1v -b true -b Off -b On --binary2 OFF --binary ON"
                           " -Bon -Bfalse -BOFF -byes";
        char ** argv;
        os << "For given source string: " << ex1 << ":" << std::endl;
        int argc = goo::dict::Configuration::tokenize_string( ex1, argv );
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << "|";
        }
        os << std::endl;

        conf.extract( argc, argv, true, &os );

        _ASSERT( conf["1"].as<bool>(),
                 "Opt -1 wasn't set." );
        {  // -b/--binary list
            auto binaryOne = conf["binary"].as_list_of<bool>();
            bool tstSeq[] = { true, false, true, true, true };
            bool * c = tstSeq;
            _ASSERT( 5 == binaryOne.size(),
                    "#1 Wrong number of parameters in list: "
                    "%d != 4.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#1 list: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // -B
            auto binaryOne = conf["B"].as_list_of<bool>();
            bool tstSeq[] = { true, false, false };
            bool * c = tstSeq;
            _ASSERT( 3 == binaryOne.size(),
                    "#2 Wrong number of parameters in list: "
                    "%d != 3.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#2 list: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // -a (must be kept default)
            auto binaryOne = conf["a"].as_list_of<bool>();
            bool tstSeq[] = { true, false, true };
            bool * c = tstSeq;
            _ASSERT( 3 == binaryOne.size(),
                    "#3 Wrong number of parameters in list: "
                    "%d != 3.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#3 list: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        //{  // -x (must be kept default)
        //    auto ushortOne = conf["x"].as_list_of<unsigned short>();
        //    unsigned short tstSeq[] = { 112, 53, 1024 };
        //    unsigned short * c = tstSeq;
        //    _ASSERT( 3 == ushortOne.size(),
        //            "#4 Wrong number of parameters in list: "
        //            "%d != 3.", (int) ushortOne.size() );
        //    for( auto it = ushortOne.begin(); ushortOne.end() != it; ++it, ++c ) {
        //        _ASSERT( *it == *c,
        //                 "#4 list: parameter #%d is set to unexpected value.",
        //                 (int) (c - tstSeq) );
        //    }
        //}

        goo::dict::Configuration::free_tokens( argc, argv );
        os << "} List parameters done." << std::endl;
    }
    # endif
    // TODO:
    // - need floating point parser to perform these tests as is;
    // - has to implement throw/catch mechanics here for consistensy checks.
    # if 0
    {
        os << "Consistency tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication3", "Testing parameter set #3." );

        conf.insertion_proxy()
            .p<bool>( 'f', "first",      "First parameter, optional one." )
            .p<bool>( 's', "second",     "Second parameter, required." )//.required()
            .list<float>( 't', "third",   "Third parameter, optional list." )
            .list<double>( '4', "fourth",  "Fourth parameter, required list." ).required()
            ;

        os << "Original config object:" << std::endl;
        conf.print_ASCII_tree( os );

        char ** argv;
        # if 0
        int argc = goo::dict::Configuration::tokenize_string( "-f ON", argv );
        conf.extract( argc, argv, true, &os );
        goo::dict::Configuration::free_tokens( argc, argv );
        # else
        const char ex1[] = "-f 12",         // `second' unset
                   ex2[] = "--second=32",   // `fourth' empty
                   ex3[] = "--second=42 -4 1.23 -4 3e+2"  // ok
                ;
        {
            // Shall throw exception as both required parameters is not set.
            int argc = goo::dict::Configuration::tokenize_string( ex1, argv );
            goo::dict::Configuration confCopy( conf );
            os << "Copy of config object:" << std::endl;
            confCopy.print_ASCII_tree( os );
            confCopy.extract( argc, argv, true, &os );
            goo::dict::Configuration::free_tokens( argc, argv );
        }
        {
            // Shall throw exception as `fourth' required parameter is not set.
            int argc = goo::dict::Configuration::tokenize_string( ex2, argv );
            goo::dict::Configuration confCopy( conf );
            confCopy.print_ASCII_tree( os );
            confCopy.extract( argc, argv, true, &os );
            goo::dict::Configuration::free_tokens( argc, argv );
        }
        {
            // Everything is ok.
            int argc = goo::dict::Configuration::tokenize_string( ex3, argv );
            goo::dict::Configuration confCopy( conf );
            confCopy.print_ASCII_tree( os );
            confCopy.extract( argc, argv, true, &os );
            goo::dict::Configuration::free_tokens( argc, argv );
        }
        # endif

        os << "} Consistency tests done." << std::endl;
    }
    # endif

    //conf.usage_text( os );

} GOO_UT_END( PDICT, "VCtr" )

