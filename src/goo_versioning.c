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

# include <stdio.h>
# include "goo_versioning.h"
# include "goo_utility.h"

const struct Goo_Versioning gooVersioning = {
    .buildTimestamp = BUILD_TIMESTAMP,
     .gitCommitHash = COMMIT_HASH,
      .gitCommitStr = GIT_STRING,
   .encodedFeatures = (0
# ifdef TYPES_128BIT_LENGTH
    | (((uint32_t) 0x1) << TYPES_128BIT_LENGTH)
# endif
# ifdef SOURCE_POSITION_INFO
    | (((uint32_t) 0x1) << SOURCE_POSITION_INFO)
# endif
# ifdef EM_STACK_UNWINDING
    | (((uint32_t) 0x1) << EM_STACK_UNWINDING)
# endif
# ifdef ANSI_ESCSEQ_PRINT
    | (((uint32_t) 0x1) << ANSI_ESCSEQ_PRINT)
# endif
    ),
/* ----------------------- */
 .encodedLibsSupport = (0
# ifdef ZLIB_FOUND
    | (((uint32_t) 0x1) << ZLIB_FOUND)
# endif
# ifdef LZO_FOUND
    | (((uint32_t) 0x1) << LZO_FOUND)
# endif
# ifdef BZIP2_FOUND
    | (((uint32_t) 0x1) << BZIP2_FOUND)
# endif
    ),
};

static const char __featuresDescrDict[][128] = {
    "128-bit types enabled",
    "source file information included to log/error messages",
    "stacktrace information can be obtained using liberty/bfd/native compiler features",
    "ANSI escape sequences enabled on shell printing",
};

static const char __libDescrDict[][128] = {
    "ZLib compression algorithm",
    "Lempel–Ziv–Oberhumer (LZO) compression algorithm",
    "BZip-2 compression algorithm",
};

void
goo_build_info( FILE * outf ) {
    fprintf( outf, "%s configuration dump:\n\r", STRINGIFY_MACRO_ARG(PACKAGE) );
    fprintf( outf, "General build info:\n\r" );
    fprintf( outf, "  version .................. : %s (%s)\n\r", STRINGIFY_MACRO_ARG(PACKAGE_VERSION),
                                                          STRINGIFY_MACRO_ARG(GOO_BUILD_TYPE));
    fprintf( outf, "  build timestamp .......... : %s\n\r", gooVersioning.buildTimestamp );
    fprintf( outf, "  git VCS commit hash ...... : %s\n\r", gooVersioning.gitCommitHash );
    fprintf( outf, "  git VCS commit string .... : %s\n\r", gooVersioning.gitCommitStr );
    fprintf( outf, "  built on ................. : %s:%s\n\r", STRINGIFY_MACRO_ARG(GOO_BUILDER_HOSTNAME),
                                                        STRINGIFY_MACRO_ARG(GOO_CMAKE_SYSTEM) );
    fprintf( outf, "  install prefix ........... : %s\n\r", STRINGIFY_MACRO_ARG(GOO_CMAKE_INSTALL_PREFIX) );
    fprintf( outf, "Misc parameters:\n\r" );
    fprintf( outf, "  emergency buffer length .. : %zd\n\r", (size_t) GOO_EMERGENCY_BUFLEN );
    fprintf( outf, "  emergency stack depth .... : %zd\n\r", (size_t) GOO_EMERGENCY_STACK_DEPTH_NENTRIES );
    fprintf( outf, "  default zlib compression . : %zd\n\r", (size_t) GOO_DEFAULT_ZLIB_COMPRESSION_LEVEL );
    fprintf( outf, "Features (0x%x):\n\r", gooVersioning.encodedFeatures );
    for( UByte n = 0; n < sizeof(__featuresDescrDict)/128; ++n ) {
        if( gooVersioning.encodedFeatures & (((uint32_t) 0x1) << n) ) {
            fprintf( outf, "  + %s\n\r", __featuresDescrDict[n] );
        }
    }
    fprintf( outf, "Libraries in use (0x%x):\n\r", gooVersioning.encodedLibsSupport );
    for( UByte n = 0; n < sizeof(__libDescrDict)/128; ++n ) {
        if( gooVersioning.encodedLibsSupport & (((uint32_t) 0x1) << n) ) {
            fprintf( outf, "  + %s\n\r", __libDescrDict[n] );
        }
    }
    fprintf( outf, "Compiler (" );
#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
    fprintf( outf, "CLang):\n\r" );
    fprintf( outf, "  version .................. : " __clang_version__ "\n\r" );
#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
    fprintf( outf, "Intel):\n\r" );
    fprintf( outf, "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
    fprintf( outf, "GNU GCC):\n\r" );
    fprintf( outf, "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */
    fprintf( outf, "HP):\n\r" );
    fprintf( outf, "  version .................. : " __HP_aCC "\n\r" );
#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
    fprintf( outf, "IBM XL):\n\r" );
    fprintf( outf, "  version .................. : " __IBMC__ "\n\r" );
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
    fprintf( outf, "MSVC):\n\r" );
    fprintf( outf, "  version .................. : " _MSC_FULL_VER "\n\r" );
    fprintf( outf, "  revision ................. : " _MSC_BUILD "\n\r" );
#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
    fprintf( outf, "PGCC):\n\r" );
    fprintf( outf, "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
    fprintf( outf, "Oracle):\n\r" );
    fprintf( outf, "  version .................. : " __SUNPRO_C "\n\r" );
#endif
    fprintf( outf, "  C compiler path .......... : " STRINGIFY_MACRO_ARG(GOO_C_COMPILER) "\n\r" );
    fprintf( outf, "  C flags .................. : " STRINGIFY_MACRO_ARG(GOO_BUILD_C_FLAGS) "\n\r" );
    fprintf( outf, "  C++ compiler path ........ : " STRINGIFY_MACRO_ARG(GOO_CXX_COMPILER) "\n\r" );
    fprintf( outf, "  C++ flags ................ : " STRINGIFY_MACRO_ARG(GOO_BUILD_CXX_FLAGS) "\n\r" );
    fprintf( outf, "  Haskell flags ............ : " STRINGIFY_MACRO_ARG(GOO_CMAKE_Haskell_FLAGS) "\n\r" );
};


