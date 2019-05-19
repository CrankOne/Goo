# include "utest.hpp"
# include "goo_mixins/iterable.tcc"

/**@file iterators.cpp
 * @brief Iterator routines test.
 * */

class SimpleConstIterator :  public goo::iterators::Iterator<
                                        goo::iterators::BaseForwardIterator,
                                        goo::iterators::BaseOutputIterator,
                                        goo::iterators::BaseComparableIterator,
                                        SimpleConstIterator, const int *, const int> {
public:
    typedef goo::iterators::Iterator<   goo::iterators::BaseForwardIterator,
                                        goo::iterators::BaseOutputIterator,
                                        goo::iterators::BaseComparableIterator,
                                        SimpleConstIterator, const int *, const int> Parent;
private:
    const int * _ptr;
public:
    SimpleConstIterator() = default;
    SimpleConstIterator( const int * ptr ) : _ptr(ptr) {}

    const int * & sym() { return _ptr; }
    const int * sym() const { return _ptr; }
};  // SimpleConstIterator



class RAIterator :  public goo::iterators::Iterator<
                                        goo::iterators::BaseBidirIterator,
                                        goo::iterators::BaseIOIterator,
                                        goo::iterators::DirectionComparableIterator,
                                        RAIterator, int *, int, size_t> {
public:
    typedef goo::iterators::Iterator<   goo::iterators::BaseBidirIterator,
                                        goo::iterators::BaseIOIterator,
                                        goo::iterators::DirectionComparableIterator ,
                                        RAIterator, int *, int, size_t> Parent;
private:
    int * _ptr;
public:
    RAIterator() = default;
    RAIterator( int * ptr ) : _ptr(ptr) {}

    int * & sym() { return _ptr; }
    const int * sym() const { return _ptr; }
};  // RAIterator


GOO_UT_BGN( Iterator, "Iterator helpers" ) {

    {  // SimpleConstIterator
        int values[20],
            k=20,
            * valuesEnd = values + sizeof(values)/sizeof(int);
        for( int * c = values; valuesEnd != c; ++c, --k ) {
            *c = k;
        }

        SimpleConstIterator it( values ),
                            end( values + sizeof(values)/sizeof(int) )
                            ;

        os << "Size of ascendants of rudimentary iterator class: "
           << "direction=" << sizeof(SimpleConstIterator::Parent::Direction) << ", "
           << "access=" << sizeof(SimpleConstIterator::Parent::Access) << ", "
           << "comparison=" << sizeof(SimpleConstIterator::Parent::Comparison) << "."
           << std::endl
           << "Size of parent of rudimentary iterator class: "
           << sizeof(SimpleConstIterator::Parent) << std::endl
           << "Size of rudimentary iterator class instance: "
           << sizeof(it) << std::endl
           ;
        k = 0;
        for( int * c = values; valuesEnd != c; ++c, ++it, ++k ) {
            _ASSERT( *c == *it, "Iterator malfunction on %d-th value: %d != %d.",
                k, *c, *it );
        }
        _ASSERT( it == end, "Iterator is not set to end element upon completion." );
    }  // SimpleConstIterator
    
    {  // RAIterator
        int values[20],
            k=20,
            * valuesEnd = values + sizeof(values)/sizeof(int);

        for( int * c = values; valuesEnd != c; ++c, --k ) {
            *c = k;
        }

        RAIterator it( values ),
                   end( values + sizeof(values)/sizeof(int) )
                   ;

        os << "Size of ascendants of random access iterator class: "
           << "direction=" << sizeof(RAIterator::Parent::Direction) << ", "
           << "access=" << sizeof(RAIterator::Parent::Access) << ", "
           << "comparison=" << sizeof(RAIterator::Parent::Comparison) << "."
           << std::endl
           << "Size of parent of random access iterator class: "
           << sizeof(RAIterator::Parent) << std::endl
           << "Size of random access iterator class instance: "
           << sizeof(it) << std::endl
           ;
        k = 0;
        for( int * c = values; valuesEnd != c; ++c, ++it, ++k ) {
            _ASSERT( *c == *it, "RA-Iterator malfunction on %d-th value: %d != %d.",
                k, *c, *it );
        }
        _ASSERT( it == end, "RA-Iterator is not set to end element upon completion." );

        RAIterator it2( values + 10 );

        _ASSERT( it != it2, "Iterator comparison failure #1." )

        _ASSERT( *it2 == 10, "Base iterator ctr failure: %d != %d (10-th element).",
            *it2, 10 );
        RAIterator it3(--it2);
        _ASSERT( *it2 == 11, "Base iterator dec #1 failure: %d != %d (9-th element).",
            *it2, 11 );
        it2--;
        _ASSERT( *it2 == 12, "Base iterator dec #2 failure: %d != %d (8-th element).",
            *it2, 12 );
        ++it2;
        _ASSERT( *it2 == 11, "Base iterator inc #1 failure: %d != %d (9-th element).",
            *it2, 11 );
        _ASSERT( it2 == it3, "Iterator comparison failure #2." );
    }  // RAIterator

} GOO_UT_END( Iterator )

