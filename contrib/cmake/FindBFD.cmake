#find_library( BFD_LIBRARY NAMES bfd PATH /usr/lib /usr/lib64 )

find_package_handle_standard_args(bfd DEFAULT_MSG BFD_LIBRARY )

include(CheckCSourceCompiles)
check_c_source_compiles(
    "#define PACKAGE Test
    #define PACKAGE_VERSION 0
  #include <bfd.h>
  int main(void) {
  return 0;
  }" BFD_WORKS)
#if( BFD_LIBRARY AND BFD_WORKS)
#    message( STATUS "Found libbfd: ${BFD_LIBRARY}")
#endif( BFD_LIBRARY AND BFD_WORKS)

