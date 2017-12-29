# include <cstring>
# include <wordexp.h>
# include "utest.hpp"
# include "goo_dict/configuration.hpp"
# include "goo_dict/parameters/enum_parameter.tcc"
# include "goo_dict/parameters/los.hpp"

/**@file pdict.cpp
 * @brief Parameters dictionary test.
 *
 * This unit test routines will perform testing of parameter dictionaries and
 * command-line parameters extraction routines supplied with Goo library. Few
 * sections test its functionality from basic to advanced usage levels.
 *
 * This module depends on virtual copy constructor routine, so VCtr has to be
 * tested first (reflected at the dependencies graph).
 * */

struct MalformedArgs {
    const char cmdLine[128];
    ErrCode expectedErrorCode;
};

static void expected_argv_parsing_error(
            const char * str,
            ErrCode ec,
            goo::dict::Configuration conf,
            std::ostream & mainOutStream ) {
    char ** argv;
    int argc = goo::dict::Configuration::tokenize_string( str, argv );
    std::stringstream subStream;
    try {
        subStream << "Input arguments after tokenization:" << std::endl;
        for( uint8_t i = 0; i < argc; ++i ) {
            subStream << "  argv[" << (int) i << "]=\""
                      << argv[i] << "\"" << std::endl;
        }
        conf.extract( argc, argv, true, &subStream );
        // The code from here to catch() part has not be evaluated unless
        // expected exception was not triggered:
        mainOutStream << "Failed case extraction log dump {" << std::endl
                      << subStream.str()
                      << "} Failed case extraction log dump" << std::endl
                      ;
        emraise( uTestFailure, "Exception not triggered when expected. "
            "Expected: \"%s\" on testing case \"%s\".",
            get_errcode_description(ec), str );
    } catch( goo::Exception & e ) {
        if( e.code() != ec ) {
            mainOutStream << "Failed case extraction log dump {" << std::endl
                      << subStream.str() << std::endl;
            mainOutStream << "Unexpected exception dump {" << std::endl;
            e.dump(mainOutStream);
            mainOutStream << "} Unexpected exception dump" << std::endl
                          << "} Failed case extraction log dump" << std::endl
                      ;
            emraise( uTestFailure, "Wrong exception triggered on testing "
                "case \"%s\". Expected: \"%s\", thrown: \"%s\".",
                str,
                get_errcode_description(ec),
                get_errcode_description(e.code()) );
        }
    }
    goo::dict::Configuration::free_tokens( argc, argv );
}

namespace other {
struct another {
enum TestingEnum {
    zero = 0,
    one = 1,
    two = 2,
    ten = 10
};  // TestingEnum
};  // struct another
}  // namespace other

# define for_all_TestingEnum_enum_entries( m, ... )   \
    m( zero,  __VA_ARGS__ ) \
    m( one,  __VA_ARGS__  ) \
    m( two,  __VA_ARGS__  ) \
    m( ten,  __VA_ARGS__  )
GOO_ENUM_PARAMETER_DEFINE( other::another:: , TestingEnum,
                            for_all_TestingEnum_enum_entries )
# undef for_all_TestingEnum_enum_entries

GOO_UT_BGN( PDICT, "Parameters dictionary routines" ) {
    # if 1
    {
        os << "Path splitting test : {" << std::endl;
        int rc;
        char    one[] = "one.three-4.five.six-seven",
                two[] = "v",
              three[] = "12one-7.1",
            * path,
            * current ;
        long index;
        {
            os << "Splitting \"" << one << "\" (common case):" << std::endl;
            _ASSERT( 0x2 == (rc = goo::dict::DictionaryParameter::pull_opt_path_token( path = one, current, index )),
                        "Wrong path subtoken #1: %d.", rc );
            _ASSERT( !strcmp("one", current) && !strcmp("three-4.five.six-seven", path),
                     "#1 was splitted to \"%s\" and \"%s\"",
                     current, path );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        {
            os << "Splitting \"" << two << "\" (single-char option remained):" << std::endl;
            _ASSERT( 0 == (rc = goo::dict::DictionaryParameter::pull_opt_path_token( path = two, current, index )),
                        "Wrong path subtoken #2: %d.", rc  );
            _ASSERT( !strcmp("v", current) && 0 == strlen(path) && 1 == (path - current),
                     "#2 was splitted to \"%s\" and string of length %d; path - current = %d",
                     current, (int) strlen(path), (int) (path - current) );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        {
            os << "Splitting \"" << three << "\" (dictionary short name):" << std::endl;
            _ASSERT( 0x2 == (rc = goo::dict::DictionaryParameter::pull_opt_path_token( path = three, current, index )),
                        "Wrong path subtoken #3: %d.", rc  );
            _ASSERT( !strcmp("12one-7", current) && !strcmp("1", path) && 1 != (path - current),
                     "#3 was splitted to \"%s\" and string of length %d",
                     current, (int) strlen(path) );
            os << "    ... current=\"" << current << "\", path=\"" << path << "\"." << std::endl;
        }

        os << "} Path splitting test done." << std::endl;
    }
    # endif
    # if 1
    {
        os << "Basic parsing tests : {" << std::endl;
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
            .p<float>( "fp-num-i", "Some fp-number" )
            .p<double>( "fp-num-ii", "Some fp-number (double)" )
            .p<std::string>( "strval", "Some string value", "defstrval" )
            .p<std::string>( 's', nullptr, "Another string value" ) 
            // ^^^ special case: setting string parameter idenitified by
            // shortcut (one-letter string) only, WITHOUT a default valuue.
            .p<other::another::TestingEnum>( 'e', "enum parameter", other::another::zero )
            ;

        const char ex1[] = "./foo -1vqfalse -simastrval --fp-num-ii 0x1.568515b1d78d4p-36 "
            " --quiet=true -V --quiet2 false -eten --verbosity 23 --fp-num-i 1.23";
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
        _ASSERT(  std::fabs(1.23 - conf["fp-num-i"].as<float>()) < 1e-6,
                  "Option fp-num-i set to wrong value (%e)",
                  conf["fp-num-i"].as<float>() );
        _ASSERT(  std::fabs(1.947e-11 - conf["fp-num-ii"].as<double>()) < 1e-24,
                  "Option fp-num-ii set to wrong value (%e)",
                  conf["fp-num-ii"].as<double>() ); 
        _ASSERT(  "defstrval" == conf["strval"].as<std::string>(),
                  "\"strval\" default set wrong: \"%s\"", conf["strval"].as<std::string>().c_str() );
        _ASSERT(  "imastrval" == conf["s"].as<std::string>(),
                  "\"s\" set wrong: \"%s\"", conf["s"].as<std::string>().c_str() );
        _ASSERT(  other::another::ten == conf["e"].as<other::another::TestingEnum>(),
                  "\"e\" set wrong: %d", (int) conf["e"].as<other::another::TestingEnum>() );

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
        os << "} Basic parsing tests done." << std::endl;
    }
    # endif
    # if 1
    {
        os << "Basic parsing errors checks: {" << std::endl;
        goo::dict::Configuration conf( "theApplication2", "Testing parameter set #1." );

        conf.insertion_proxy()
            .flag(    '1',  "A logic flag that may be set." )
            .p<bool>( 'a',  "A logic option that may be set.", true )
            .p<char>( 'b',  "A <char> type parameter" )
            .p<uint8_t>( 'B', "bee", "A <unsigned char> type parameter" )
            .p<float>( "flt", "A <float> parameter" )
            ;
        MalformedArgs mlfrmdArgs[] = {
            { "foo -1yes",              Exception::parserFailure },  // cmd-line arg isn't recognized
            { "foo -a=stopme",          Exception::parserFailure },  // unable to parse
            { "foo -b 15blah",          Exception::parserFailure },  // extra symbols on tail
            { "foo -b 129",             Exception::overflow },
            { "foo -B -1",              Exception::underflow },
            { "foo -B=1",               Exception::parserFailure },  // unable to parse =1 as number
            //{ "foo --flt=0x1.568515b1d78d4p-72", Exception::narrowConversion }, TODO: how to implement?
            // ... more!
            { "",                      Exception::common}
        };

        for( auto c = mlfrmdArgs; '\0' != c->cmdLine[0]; ++c ) {
            expected_argv_parsing_error( c->cmdLine,
                c->expectedErrorCode, conf, os );
        }

        os << "} Basic parsing errors checks." << std::endl;
    }
    # endif
    # if 1
    {
        os << "List parameters tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication3", "Testing parameter set #2." );

        const char strLstDftCheck[][16] = { "one", "two", "three" };

        conf.insertion_proxy()
                .flag( '1', "parameter-one", "First parameter" )
                .array<bool>( 'b', "binary", "options array #1", {true, true, false, false} )
                .array<bool>( 'B', "options array #2", {false, false, false, false} )
                .array<bool>( "binary2", "options array #3" )
                .array<bool>( 'a', "options array #4", {true, false, true} )
                .flag( 'v', "Enables verbose output" )
                .array<short>( 'x', "List of short ints", {112, 53, 1024} )
                .array<float>( "fl-num", "List of floating numbers" )
                .array<std::string>( 's', "lstr-i", "String array one", {"a", "b", "c"} )
                .array<std::string>( "lstr-ii", "String array two", {"one", "two", "three"} )
                .array<std::string>( 'S', nullptr, "String array three", {"1", "two", "III"} )
            ;

        const char ex1[] = "./foo -s one -1v --fl-num 1e-6 -b true -b Off -b On "
                           "--binary2 OFF -stwo --binary ON -Bon -Bfalse -BOFF "
                           "-byes -s three";
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
        {  // -b/--binary array
            auto binaryOne = conf["binary"].as_array_of<bool>();
            bool tstSeq[] = { true, false, true, true, true };
            bool * c = tstSeq;
            _ASSERT( 5 == binaryOne.size(),
                    "#1 Wrong number of parameters in array: "
                    "%d != 4.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#1 array: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // -B
            auto binaryOne = conf["B"].as_array_of<bool>();
            bool tstSeq[] = { true, false, false };
            bool * c = tstSeq;
            _ASSERT( 3 == binaryOne.size(),
                    "#2 Wrong number of parameters in array: "
                    "%d != 3.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#2 array: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // -a (must be kept default)
            auto binaryOne = conf["a"].as_array_of<bool>();
            bool tstSeq[] = { true, false, true };
            bool * c = tstSeq;
            _ASSERT( 3 == binaryOne.size(),
                    "#3 Wrong number of parameters in array: "
                    "%d != 3.", (int) binaryOne.size() );
            for( auto it = binaryOne.begin(); binaryOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#3 array: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // -x (must be kept default)
            auto ushortOne = conf["x"].as_array_of<short>();
            unsigned short tstSeq[] = { 112, 53, 1024 };
            unsigned short * c = tstSeq;
            _ASSERT( 3 == ushortOne.size(),
                    "#4 Wrong number of parameters in array: "
                    "%d != 3.", (int) ushortOne.size() );
            for( auto it = ushortOne.begin(); ushortOne.end() != it; ++it, ++c ) {
                _ASSERT( *it == *c,
                         "#4 array: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // --fl-num (must have one element ~1e-6)
            auto fltOne = conf["fl-num"].as_array_of<float>();
            float tstSeq[] = { 1e-6f };
            float * c = tstSeq;
            _ASSERT( 1 == fltOne.size(),
                    "#5 Wrong number of parameters in array: "
                    "%d != 1.", (int) fltOne.size() );
            for( auto it = fltOne.begin(); fltOne.end() != it; ++it, ++c ) {
                _ASSERT( std::fabs(*it - *c) < 1e-7,
                         "#5 array: parameter #%d is set to unexpected value.",
                         (int) (c - tstSeq) );
            }
        }
        {  // --lstr-i has to be array of strings:
            auto strOne = conf["lstr-i"].as_array_of<std::string>();
            const char (*c)[16] = strLstDftCheck;
            _ASSERT( 3 == strOne.size(),
                    "#6 Wrong number of parameters in array: "
                    "%d != 1.", (int) strOne.size() );
            for( auto it = strOne.begin(); strOne.end() != it; ++it, ++c ) {
                _ASSERT( !strcmp( *c, it->c_str() ),
                         "#6 array: parameter #%d is set to unexpected value: %s.",
                         (int) (c - strLstDftCheck), it->c_str() );
            }
        }
        {  // --lstr-ii has to be array of strings:
            auto strTwo = conf["lstr-ii"].as_array_of<std::string>();
            const char (*c)[16] = strLstDftCheck;
            _ASSERT( 3 == strTwo.size(),
                    "#7 Wrong number of parameters in array: "
                    "%d != 1.", (int) strTwo.size() );
            for( auto it = strTwo.begin(); strTwo.end() != it; ++it, ++c ) {
                _ASSERT( !strcmp( *c, it->c_str() ),
                         "#7 array: parameter #%d is set to unexpected value: %s.",
                         (int) (c - strLstDftCheck), it->c_str() );
            }
        }

        goo::dict::Configuration::free_tokens( argc, argv );
        os << "} List parameters done." << std::endl;
    }
    # endif
    // todo:
    // ✔ need floating point parser to perform these tests as is;
    // ✔ has to implement throw/catch mechanics here for consistensy checks.
    # if 1
    {
        os << "Consistency tests : {" << std::endl;
        goo::dict::Configuration conf( "theApplication4", "Testing parameter set #3." );

        conf.insertion_proxy()
            .p<int>( 'f', "first",          "First parameter, optional one." )
            .p<bool>( 's', "second",        "Second parameter, required." ).required_argument()
            .array<float>( 't', "third",     "Third parameter, optional array." )
            .array<double>( '4', "fourth",   "Fourth parameter, required array." ).required_argument()
            ;

        os << "Original config object:" << std::endl;
        conf.print_ASCII_tree( os );

        char ** argv;
        # if 0
        int argc = goo::dict::Configuration::tokenize_string( "blah blah", argv );
        conf.extract( argc, argv, true, &os );
        goo::dict::Configuration::free_tokens( argc, argv );
        # else
        const char ex3[] = "three --second=no -4 1.23 -4 3e+2";
        MalformedArgs mlfrmdArgs[] = {
            { "one --second=yes",                   Exception::inconsistentConfig },  // `fourth' empty
            { "two -f 12",                          Exception::inconsistentConfig },  // `second' unset
            //{ "three --second=no -4 1.23 -4 3e+2",  Exception::common }  // ok
            { "", Exception::common }
        };
        for( auto c = mlfrmdArgs; '\0' != c->cmdLine[0]; ++c ) {
            expected_argv_parsing_error( c->cmdLine,
                c->expectedErrorCode, conf, os );
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
    # if 1
    {
        os << "Subsection parameter retrieving : {" << std::endl;
        goo::dict::Configuration conf( "theApplication5", "Testing parameter set #3." );

        conf.insertion_proxy()
            .p<unsigned char>( 'v', "verbosity",    "Verbosity level." )
            .flag( 'q', "quiet",                    "Be absolutely quiet." )
            .bgn_sect( "subsect1", "Subsection #1" )
                .p<float>( "sub-parameter-one", "Some scoped parameter #1" )
                .p<bool>(  '2', "sub-parameter-two", "Another scoped parameter" )
                .bgn_sect( "subsect2", "Subsection #2" )
                    .p<uint32_t>( "sub-parameter-one", "Some scoped parameter #2" )
                .end_sect( "subsect2" )
            .end_sect( "subsect1" )
            .bgn_sect( "subsect3", "Subsection #3" )
                .p<int16_t>( "sub-parameter-one", "Some scoped parameter #3" )
                .flag( "imaflag", "A scoped flag" )
            .end_sect( "subsect3" )
            ;

        conf.print_ASCII_tree( os );
        const char ex[] = "foo -v3 -q --subsect1.subsect2.sub-parameter-one=5673356 "
            "--subsect1.sub-parameter-one 1.23 --subsect3.sub-parameter-one -12 "
            "--subsect3.imaflag"
            ;
        char ** argv;
        int argc = goo::dict::Configuration::tokenize_string( ex, argv );
        os << "  ->";
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << "|";
        } os << std::endl;

        os << "} Subsection parameter retrieving." << std::endl;
        conf.extract( argc, argv, true, &os );
        goo::dict::Configuration::free_tokens( argc, argv );

        _ASSERT( conf["v"].as<uint8_t>() == 3, "'v' parameter set wrong" );
        _ASSERT( conf["quiet"].as<bool>(), "\"quiet\" parameter set wrong" );
        _ASSERT( 1e-6 > std::fabs(conf["subsect1.sub-parameter-one"].as<float>() - 1.23),
                    "\"subsect1.sub-parameter-one\" set wrong.");
        _ASSERT( conf["subsect1.subsect2.sub-parameter-one"].as<uint32_t>() == 5673356,
                    "\"subsect1.subsect2.sub-parameter-one\" set wrong" )
        _ASSERT( conf["subsect3.sub-parameter-one"].as<int16_t>() == -12,
                    "\"subsect3.sub-parameter-one\" set wrong")
        _ASSERT( conf["subsect3.imaflag"].as<bool>(),
                    "\"subsect3.imaflag\" set wrong")
    }
    # endif
    # if 1
    // See http://unix.stackexchange.com/questions/147143/when-and-how-was-the-double-dash-introduced-as-an-end-of-options-delimiter
    // for historical details about useful `--` special token that initially
    // appeared as a kludge to further become a standard.
    {
        os << "End-of-options test : {" << std::endl;
        goo::dict::Configuration conf( "theApplication6", "Testing parameter set #4." );

        conf.positional_arguments<std::string>( "exec-cmd-argv",
                "All positional arguments will be forwarded to child "
                "invokation in order of appearance." );

        conf.insertion_proxy()
                .bgn_sect( "log", "Logging options" )
                .array<std::string>( '1', "stdout", "List of files where stdout has to be "
                        "redirected. The '-' will be considered as "
                        "terminal's stdout if available.", {"-"} )
                .array<std::string>( '2', "stderr", "List of files where stderr has to be "
                        "redirected. The '-' will be considered as "
                        "terminal's stderr if available.", {"-"} )
            .end_sect( "log" )
            .flag(  "emulate-tty", "If provided, the fake TTY will be injected "
                    "into child process' environment." )
            ;

        conf.print_ASCII_tree( os );
        const char ex[] = "extended-tee cmake -1/tmp/build.log -2/tmp/errors.log "
            "-1/tmp/build-11-03-017.log -2/tmp/build-11-03-017.log -1- -2- -- "
            "-1 some -DSOMPARAM=OFF -2 thing ../some/where --foo --bar"
            ;
        char ** argv;
        int argc = goo::dict::Configuration::tokenize_string( ex, argv );
        os << "  ->";
        for( int n = 0; n < argc; ++n ) {
            os << argv[n] << "|";
        } os << std::endl;

        conf.extract( argc, argv, true, &os );
        goo::dict::Configuration::free_tokens( argc, argv );

        const char chkTokens[][10][64] = {
            { "/tmp/build.log",     "/tmp/build-11-03-017.log", "-", "" },
            { "/tmp/errors.log",    "/tmp/build-11-03-017.log", "-", "" },
            { "cmake", "-1", "some", "-DSOMPARAM=OFF", "-2", "thing",
                                    "../some/where", "--foo", "--bar", "" },
        };

        {
            const char (* tok1)[64] = chkTokens[0];
            for( auto stoFName : conf["log.stdout"].as_array_of<std::string>() ) {
                os << "  stdout redirected to: " << stoFName << std::endl;
                _ASSERT( !strcmp( *tok1, stoFName.c_str() ),
                    "Positional argument #%d failure on array 1: \"%s\" instead of "
                    "expected \"%s\".", (int) (chkTokens[0] - tok1), 
                                        stoFName.c_str(), *tok1 );
                tok1++;
            }
            _ASSERT( '\0' == tok1[0][0], "Missing argument for array 1: \"%s\".",
                *tok1 );
        }

        {
            const char (* tok2)[64] = chkTokens[1];
            for( auto stoFName : conf["log.stderr"].as_array_of<std::string>() ) {
                os << "  stderr redirected to: " << stoFName << std::endl;
                _ASSERT( !strcmp( *tok2, stoFName.c_str() ),
                    "Positional argument #%d failure on array 2: \"%s\" instead of "
                    "expected \"%s\".", (int) (chkTokens[1] - tok2), 
                                        stoFName.c_str(), *tok2 );
                tok2++;
            }
            _ASSERT( '\0' == tok2[0][0], "Missing argument for array 2: \"%s\".",
                *tok2 );
        }

        {
            const char (* tok3)[64] = chkTokens[2];
            os << " command to run: $";
            for( auto fwdArgv : conf["exec-cmd-argv"].as_array_of<std::string>() ) {
                os << " " << fwdArgv;
                _ASSERT( !strcmp( *tok3, fwdArgv.c_str() ),
                    "Positional argument #%d failure on array 3: \"%s\" instead of "
                    "expected \"%s\".", (int) (chkTokens[2] - tok3), 
                                        fwdArgv.c_str(), *tok3 );
                tok3++;
            }
            os << std::endl;
            _ASSERT( '\0' == tok3[0][0], "Missing argument for array 3: \"%s\".",
                *tok3 );
        }

        os << "} End-of-options test done." << std::endl;
    }
    # endif
    # if 1
    // There is no way to define application configuration in a way that allows
    // construction of AoS-like structures. However goo::dict allows it in a
    // peculiar way. Here is a test case of constructing such structure and
    // retrieving  elements from it. The practical rationale for this is to provide
    // native compatibility with similar structures in elaborated mark-up languages
    // such as YAML or XML.
    {
        //typedef goo::dict::Parameter<goo::dict::List<goo::dict::iAbstractParameter * > > AoS;
        goo::dict::DictionaryParameter d("object", "Aggregating object.")
                                     //, subD1("subObject1", "Aggregated object #1.")
                                     //, subD2("subObject2", "Aggregated object #2.")
                                     ;

        // In comments in block below following notation is used:
        // LIP -- list insertion proxy
        // DIP -- dict insertion proxy
        // ?IP's -- means, that method is defined in ? class.
        d.insertion_proxy().bgn_list( "aos", "Testing array of structures." )  //< DIP's, returns LIP
                .v( 1 )
                .v( std::string("two") )
                .v( 3. )
                .bgn_dict()  //< LIP's, returns DIP
                        .p<int>( "some-integer", "Integer parameter without a value." ).required_argument()
                        .bgn_sect( "subsect", "Sub-section within a dict inside array of objects." )  //< DIP's -> DIP
                            .p<float>( 'f', "float value" )
                        .end_sect( "subsect" )  //< DIP's -> DIP
                        .bgn_list( "empty-list", "Annotated empty list." )  //< DIP's -> LIP
                        .end_list( "empty-list" )  //< LIP's -> DITP
                .end_dict()  //< DIP's, returns LIP
                .v( std::string("five") )
                .bgn_sublist()  //< LIP's, returns LIP
                    .v( std::string("six") )
                    .v( 7. )
                .end_sublist()  //< LIP's, returns LIP
            .end_list()  // LIP's -> DIP
        ;
        // ...
    }
    # endif

    //conf.usage_text( os );

} GOO_UT_END( PDICT, "VCtr" )

