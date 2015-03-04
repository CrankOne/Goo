# if 1

# include <cstring>
# include <ext/pool_allocator.h>
# include <vector>
# include "utest.hpp"
# include "hph_exception.hpp"
//# include "hph_buffer.hpp"
# include "hph_allocator.hpp"

int tstSeq[] = {1,2,3};
struct TestingStruct {
    void * somePtr;
    int foo;
    double bar;
};
const char tstNames[][32] = {
    "Anneliese",        "Brunhild",     "Bärbel",
    "Edeltraud",        "Gerlinde",     "Hanna",
    "Irmhild",          "Kriemhild",    "Ludwiga",
    "Mathilde",         "Pauline",      "Ulla"
};
struct CompositeStruct {
    int _a,
        _b,
        * _c;
    CompositeStruct() = delete;
    CompositeStruct(int a, int b) : _a(a), _b(b) { _c = new int; }
    virtual ~CompositeStruct() { delete _c; }
};

void run_test_on_allocator( hph::mem::iAllocator * A,
                            std::ostream & out ) {
    { out << "  Simple allocator operations." << std::endl;
      auto cObj = A->alloc<CompositeStruct>(1, 0, 11, 42);
      _ASSERT( cObj->_a == 11 && cObj->_b == 42, "Compound type ctr malfunction." );
      A->del<CompositeStruct>( cObj, 1);
      out << "    ...passed." << std::endl;
    }
    { out << "  Atomic types within STL container." << std::endl;
        std::vector<int, hph::mem::iAllocator::STLCompliantAllocator<int> >
            myvec( *A );
        myvec.reserve(sizeof(tstSeq)/sizeof(int));
        for( UByte i = 0; i < sizeof(tstSeq)/sizeof(int); ++i ) {
            myvec.push_back(tstSeq[i]);
        }
        for( UByte i = 0; i < sizeof(tstSeq)/sizeof(int); ++i ) {
            _ASSERT( myvec[i] == tstSeq[i],
                     "Data corrupted in STL container #1." );
        }
      out << "    ...passed." << std::endl;
    }
    { out << "  STL String type." << std::endl;
        hph::AString * strv = A->alloc<hph::AString>(1, nullptr, "Blah.",
                                         hph::mem::iAllocator::STLCompliantAllocator<char>(*A));
        *strv = "Blah-blah. Blah.";

        std::vector<hph::AString, hph::mem::iAllocator::STLCompliantAllocator<hph::AString> >
                       vec(*A);
        vec.push_back( *strv );
        vec.push_back( hph::AString("one", hph::mem::iAllocator::STLCompliantAllocator<char>(*A)) );
        vec.push_back( hph::AString("two", *A) );
        A->del<hph::AString>( strv, 1 );
        out << "    ...passed." << std::endl;
    }
    # if 1
    { out << "  Compound types within STL container." << std::endl;
        std::map<hph::AString, TestingStruct,
                 std::less<hph::AString>,
                 hph::mem::iAllocator::STLCompliantAllocator<
                    std::pair<hph::AString, TestingStruct> > >
                        tstMap( std::less<hph::AString>(), *A );
        for( UByte i = 0; i < sizeof(tstNames)/32; ++i ) {
            tstMap.insert( std::pair<hph::AString, TestingStruct>(
                    hph::AString(tstNames[i], *A ),
                    {(char*) (tstNames + i), i, float(i)/32 }) );
        }
        UByte i = 0;
        for( auto it = tstMap.begin(); it != tstMap.end(); ++it, ++i ) {
            _ASSERT( it->first == tstNames[i] &&
                     tstNames+i == it->second.somePtr,
                    "Data corrupted in STL container #2: %s != %s.",
                     it->first.c_str(), tstNames[i]);
            //        
        }
      out << "    ...passed." << std::endl;
    }
    # endif
}

typedef hph::mem::STLAllocator<__gnu_cxx::__pool_alloc> PoolAllocator;

HPH_UT_BGN( Allocators )

{
    out << "Default allocator:" << std::endl;
    run_test_on_allocator( hph::mem::get_default_allocator(), out );
    {
        out << "GCC's pool allocator:" << std::endl;
        PoolAllocator pa;
        run_test_on_allocator( &pa, out );
    }
}

HPH_UT_END( Allocators )

    # if 0
    LazyAllocator A;
    _ASSERT( A.n_blocks_bound() == 0 && A.n_blocks_unbound() == 0,
        "void allocr shows itself as non-empty"  );
    Size * ref1 = null;
    UByte * data1 = A.alloc( 12, ref1 );
    _ASSERT( A.n_blocks_bound() == 1 && A.n_blocks_unbound() == 0,
        "one block allocation assertion failed"  );
    Size * ref2 = null;
    UByte * data2 = A.alloc( 11, ref2 );
    _ASSERT( A.n_blocks_bound() == 2 && A.n_blocks_unbound() == 0,
        "two block allocation assertion failed"  );
    Size * ref3 = null;
    UByte * data3 = A.alloc( 13, ref3 );
    _ASSERT( A.n_blocks_bound() == 3 && A.n_blocks_unbound() == 0,
        "three block allocation assertion failed"  );

    A.free(data3);
    _ASSERT( A.n_blocks_bound() == 2 && A.n_blocks_unbound() == 1,
        "one block freeing assertion failed (" HPH_SIZE_FMT ", " HPH_SIZE_FMT ")",
                    A.n_blocks_bound(),
                    A.n_blocks_unbound()  );

    data3 = A.alloc( 12, ref3 );
    _ASSERT( A.n_blocks_bound() == 3 && A.n_blocks_unbound() == 0,
        "conservative assertion failed"  );

    A.force_free(data1);
    _ASSERT( A.n_blocks_bound() == 2 && A.n_blocks_unbound() == 0,
        "force free assertion failed (" HPH_SIZE_FMT ", " HPH_SIZE_FMT ")",
                    A.n_blocks_bound(),
                    A.n_blocks_unbound() );
    A.free(data2);
    A.free(data3);

    _ASSERT( A.n_blocks_bound() == 0 && A.n_blocks_unbound() == 2,
        "invalid number of unbound blocks (" HPH_SIZE_FMT ", " HPH_SIZE_FMT ")",
                    A.n_blocks_bound(),
                    A.n_blocks_unbound()  );

    A.clear_unbound();
    _ASSERT( A.n_blocks_bound() == 0 && A.n_blocks_unbound() == 0,
        "has unbound block while it had not to (" HPH_SIZE_FMT ", " HPH_SIZE_FMT ")",
                    A.n_blocks_bound(),
                    A.n_blocks_unbound()  );
    # endif
# endif

