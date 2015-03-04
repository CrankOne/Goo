find_library( BFD_LIB	    NAMES bfd	    PATH /usr/lib /usr/lib64 )

include(CheckCSourceCompiles)
check_c_source_compiles(
    "#define PACKAGE Test
    #define PACKAGE_VERSION 0
  #include <bfd.h>
  int main(void) {
  return 0;
  }" BFD_WORKS)
if( BFD_LIB AND BFD_WORKS)
	set( BFD_FOUND TRUE )
endif( BFD_LIB AND BFD_WORKS)

if( BFD_FOUND )
	message( STATUS "Found libbfd: ${BFD_LIB}")
endif( BFD_FOUND )
