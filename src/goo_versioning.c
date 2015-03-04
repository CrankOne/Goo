# include <stdio.h>
# include "goo_versioning.h"
# include "goo_utility.h"

const struct HPH_Versioning hphVersioning = {
    .buildTimestamp = BUILD_TIMESTAMP,
     .gitCommitHash = COMMIT_HASH,
      .gitCommitStr = GIT_STRING,
   .encodedFeatures = (0
# ifdef EXTENDED_TYPES
    |   EXTENDED_TYPES
# endif
# ifdef PRINTF_SOURCE_INFO
    |   PRINTF_SOURCE_INFO
# endif
# ifdef EXCEPTION_BACTRACE
    |   EXCEPTION_BACTRACE
# endif
# ifdef ENABLE_GDS
    |   ENABLE_GDS
# endif
# ifdef ENABLE_ALLOCATORS
    |   ENABLE_ALLOCATORS
# endif
# ifdef ENABLE_TENSORS
    |   ENABLE_TENSORS
# endif
# ifdef ENABLE_DATASTREAMS
    |   ENABLE_DATASTREAMS
# endif
# ifdef ENABLE_PTREE
    |   ENABLE_PTREE
# endif
# ifdef HASKELL_MODULE
    |   HASKELL_MODULE
# endif
    ),
/* ----------------------- */
 .encodedLibsSupport = (0
# ifdef ZLIB_FOUND
    |   ZLIB_FOUND
# endif
# ifdef LZO_FOUND
    |   LZO_FOUND
# endif
# ifdef BZIP2_FOUND
    |   BZIP2_FOUND
# endif
# ifdef PNG_FOUND
    |   PNG_FOUND
# endif
# ifdef FLTK_FOUND
    |   FLTK_FOUND
# endif
# ifdef GIT_FOUND
    |   GIT_FOUND
# endif
    ),
};

static const char __featuresDescrDict[][128] = {
    "128-bit types enabled"
    "source file information included to hph-standard log messages",
    "stacktrace information will be obtained when hph::Exception is thrown",
    "Goo Declarative Semantics language",
    "instance-based allocators subsystem",
    "tensorial algebra extensions are supported",
    "Goo's serialization streams protocols are supported",
    "parameters tree supported",
    "Haskell module is embedded in library"
};

static const char __libDescrDict[][128] = {
    "ZLib compression algorithm",
    "Lempel–Ziv–Oberhumer (LZO) compression algorithm",
    "BZip-2 compression algorithm",
    "PNG image",
    "FLTK GUI library",
    "GIT versioning support",
};

void
build_info() {
    printf( "%s configuration dump:\n", STRINGIFY_MACRO_ARG(PACKAGE) );
    printf( "General build info:\n\r" );
    printf( "  version .................. : %s (%s)\n\r", STRINGIFY_MACRO_ARG(PACKAGE_VERSION),
                                                          STRINGIFY_MACRO_ARG(BUILD_TYPE));
    printf( "  build timestamp .......... : %s\n\r", hphVersioning.buildTimestamp );
    printf( "  git VCS commit hash ...... : %s\n\r", hphVersioning.gitCommitHash );
    printf( "  git VCS commit string .... : %s\n\r", hphVersioning.gitCommitStr );
    printf( "  built on ................. : %s:%s\n\r", STRINGIFY_MACRO_ARG(BUILDER_HOSTNAME),
                                                        STRINGIFY_MACRO_ARG(CMAKE_SYSTEM) );
    printf( "Misc parameters:\n\r" );
    printf( "  emergency buffer length .. : %zd\n\r", (size_t) EMERGENCY_BUFLEN );
    printf( "  emergency stack depth .... : %zd\n\r", (size_t) EMERGENCY_STACK_DEPTH_NENTRIES );
    printf( "  default zlib compression . : %zd\n\r", (size_t) DEFAULT_ZLIB_COMPRESSION_LEVEL );
    printf( "Features (0x%x):\n\r", hphVersioning.encodedFeatures );
    for( UByte n = 0; n < sizeof(__featuresDescrDict)/128; ++n ) {
        if( hphVersioning.encodedFeatures & (((uint32_t) 1) << n) ) {
            printf( "  - %s\n\r", __featuresDescrDict[n] );
        }
    }
    printf( "Libraries in use (0x%x):\n\r", hphVersioning.encodedLibsSupport );
    for( UByte n = 0; n < sizeof(__libDescrDict)/128; ++n ) {
        if( hphVersioning.encodedLibsSupport & (((uint32_t) 1) << n) ) {
            printf( "  - %s\n\r", __libDescrDict[n] );
        }
    }
    printf( "Compiler (" );
#if defined(__clang__)
	/* Clang/LLVM. ---------------------------------------------- */
    printf( "CLang):\n\r" );
    printf( "  version .................. : " __clang_version__ "\n\r" );
#elif defined(__ICC) || defined(__INTEL_COMPILER)
	/* Intel ICC/ICPC. ------------------------------------------ */
    printf( "Intel):\n\r" );
    printf( "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__GNUC__) || defined(__GNUG__)
	/* GNU GCC/G++. --------------------------------------------- */
    printf( "GNU GCC):\n\r" );
    printf( "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__HP_cc) || defined(__HP_aCC)
	/* Hewlett-Packard C/aC++. ---------------------------------- */
    printf( "HP):\n\r" );
    printf( "  version .................. : " __HP_aCC "\n\r" );
#elif defined(__IBMC__) || defined(__IBMCPP__)
	/* IBM XL C/C++. -------------------------------------------- */
    printf( "IBM XL):\n\r" );
    printf( "  version .................. : " __IBMC__ "\n\r" );
#elif defined(_MSC_VER)
	/* Microsoft Visual Studio. --------------------------------- */
    printf( "MSVC):\n\r" );
    printf( "  version .................. : " _MSC_FULL_VER "\n\r" );
    printf( "  revision ................. : " _MSC_BUILD "\n\r" );
#elif defined(__PGI)
	/* Portland Group PGCC/PGCPP. ------------------------------- */
    printf( "PGCC):\n\r" );
    printf( "  version .................. : " __VERSION__ "\n\r" );
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
	/* Oracle Solaris Studio. ----------------------------------- */
    printf( "Oracle):\n\r" );
    printf( "  version .................. : " __SUNPRO_C "\n\r" );
#endif
    printf( "  C flags .................. : " STRINGIFY_MACRO_ARG(BUILD_C_FLAGS) "\n\r" );
    printf( "  C++ flags ................ : " STRINGIFY_MACRO_ARG(BUILD_CXX_FLAGS) "\n\r" );
    printf( "  Haskell flags ............ : " STRINGIFY_MACRO_ARG(CMAKE_Haskell_FLAGS) "\n\r" );
};


