/** Includes following classes:
 *
 * 1. Simple classes representing single parameter:
 *      ParameterBase -- an abstract base for each parameter. Always should
 *                       contain a textual representation of parameter. Also
 *                       stores a textual description of parameter for users.
 *      Parameter<T> <- ParameterBase -- a template class for parameter of
 *                      particular arithmetic/string type. Raises an exception,
 *                      if getter is applied to uninitialized parameter.
 * 2. Parameters set:
 *      ParameterBranch <- std::unordered_map<std::string, ParameterBase *> -- a subset
 *                      of parameters; is an associative array of pairs (name -> parameter
 *                      instance).
 *      ParameterSet <- ParameterBranch -- a high level wrapper to PArameterBranch that user
 *                      is supposed to deal with. Provides recursive single-direction iterator
 *                      class, some aux special template methods like parse_stream(), etc.
 * */
# include "dict/dict.tcc"

int
main(int argc, char * argv[]){
    // 1. Construct an empty parameter set dictionary.
    goo::dicts::Dictionary dict;

    // 2. Declare a supposed structure of parameters.
    dict.get_insertion_proxy()  // returns a reference to special object -- insertion proxy.
        .p<int>(     "optLevel",        "frobonication optimization level" )  // adds an uninitialized int parameter with
                                                                              // name /optLevel and returns insertion proxy reference.
        .p<float>(   "distance",        "distance to frobonicate with" )  // same, for type float and name 'distance'
        .p<bool>(    "applyFilters",    "controls, do we need to apply filters after glorbs are frobonicated?" )
        .bgn_sect("presets", "some default presets")        // creates subtree and returns insertion proxy to it
            .p<std::string>("uploadsDir",   "directory to upload files in", "/tmp/uploads")  // parameter with default value
            .p<int>(        "color",        "RGB color of glorbification",  0x6e6e6e )
        .end_sect()
        .p<>(               "enableFrobonication",  "do we need frobonication?", false )  // C++'s automatic type deduction used.
        .p<std::string>(    "files",                "files to glorbify")
        .arr<std::string>(  "templates",            "glorbification templates", {"~/one", "~/two", "~/three"});

    // 3. Parse config file.
    dict.parse_stream( jsonParser(), std::ofstream("config.json") );

    // 4. Merge parameter set with command line parameters.
    try {
        dict.parse( argc, argv );
    } catch( goo::Exception & e ) {  // Usual exception stuff that's already implemented in Goo.
        eprintf( "Got an error whithin cmd-line parsing routine:\n" );
        e.print_error( std::cerr );
        dict.print_usage( std::cerr );  // <-- For THIS we have a textual descriptions.
        return EXIT_FAILURE;
    }


    // 5. Obtain parameters.
    int oLvl    = dict( "optLevel" ).as<int>(),
        color   = dict( "presets/color" ).as<int>();
    
    // ...

    return EXIT_SUCCESS;
}
