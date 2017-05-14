# include "utest.hpp"
# include "goo_mixins/iterable.tcc"

/**@file iterators.cpp
 * @brief Iterator routines test.
 * */

class TestingArray {
private:
    int * _valuesPtr;
};

class RudimentaryIterator : public goo::iterators::Iterator<
        goo::iterators::BaseForwardIterator,
        goo::iterators::BaseOutputIterator,
        goo::iterators::BaseComparableIterator,
        RudimentaryIterator, int *, int> {
public:
    typedef goo::iterators::Iterator<
                goo::iterators::BaseForwardIterator,
                goo::iterators::BaseOutputIterator,
                goo::iterators::BaseComparableIterator,
                RudimentaryIterator, int *, int> Parent;
protected:
    virtual const Value & _V_dereference( int * const & s ) const override {
        return *s;
    }
    virtual bool _V_compare( int * const & s ) const override {
        return s != sym();
    }
    virtual void _V_increment( ) override {
        ++sym();
    }
public:
    RudimentaryIterator() : ::goo::iterators::BaseIterator<int *>(nullptr) {}
    RudimentaryIterator(int * s) : ::goo::iterators::BaseIterator<int *>(s) {}
};

GOO_UT_BGN( Iterator, "Iterator helpers" ) {

    {
        int values[20],
            k=20,
            * valuesEnd = values + sizeof(values)/sizeof(int);
        for( int * c = values; valuesEnd != c; ++c, --k ) {
            *c = k;
        }

        RudimentaryIterator it( values ),
                            end( values + sizeof(values)/sizeof(int) )
                            ;

        os << "Size of BaseIterator class: "
           << sizeof(::goo::iterators::BaseIterator<int *>) << std::endl
           << "Size of ascendants of rudimentary iterator class: "
           << "direction=" << sizeof(RudimentaryIterator::Parent::Direction) << ", "
           << "access=" << sizeof(RudimentaryIterator::Parent::Access) << ", "
           << "comparison=" << sizeof(RudimentaryIterator::Parent::Comparison) << "."
           << std::endl
           << "Size of parent of rudimentary iterator class: "
           << sizeof(RudimentaryIterator::Parent) << std::endl
           << "Size of rudimentary iterator class instance: "
           << sizeof(it) << std::endl
           ;
        k = 0;
        for( int * c = values; valuesEnd != c; ++c, ++it, ++k ) {
            _ASSERT( *c == *it, "Iterator malfunction on %d-th value: %d != %d.",
                k, *c, *it );
        }
    }

} GOO_UT_END( Iterator )

