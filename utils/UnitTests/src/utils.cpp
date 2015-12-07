# include <cstring>
# include "utest.hpp"
# include "goo_utility.h"

GOO_UT_BGN( utils, "Goo utilities test" )

{
    const char target[]  = "Peter Piper picked a peck of pickled peppers;\n"
                           "A peck of pickled peppers Peter Piper picked;\n"
                           "If Peter Piper picked a peck of pickled peppers,\n"
                           "Where's the peck of pickled peppers Peter Piper picked?";
      char templ[1024]   = "${person} ${action} a ${item};\n"
                           "A ${item} ${person} ${action};\n"
                           "If ${person} ${action} a ${item},\n"
                           "Where's the ${item} ${person} ${action}?"
                           ;
    char buffer[1024] = "\0"; 
    _ASSERT( 0 < replace_string( templ,  "${person}", "Peter Piper", buffer, sizeof(buffer) ),
        "Substitution #1." );
    _ASSERT( 0 < replace_string( buffer, "${action}", "picked",      templ,  sizeof(templ)  ),
        "Substitution #2." );
    _ASSERT( 0 < replace_string( templ, "${item}", "peck of pickled peppers", buffer, sizeof(buffer) ),
        "Substitution #3." );
    os << buffer << std::endl;
    _ASSERT( 0 == strcmp( target, buffer ), "Substitution failed: strings aren't identical." );
}

GOO_UT_END( utils )

