# include "utest.hpp"
# include "hph_iterators.tcc"

// ++, +=, --, -=, +, -

class IterableConsumerBidir : public hph::IterableTraits<
            size_t,
            size_t,
            size_t,
            hph::mixins::RandomForwardIterable,
            hph::mixins::RandomBackwardIterable
        >::Base {
public:
    typedef hph::IterableTraits<
                size_t,
                size_t,
                size_t,
                hph::mixins::RandomForwardIterable,
                hph::mixins::RandomBackwardIterable>     Traits;
    typedef typename Traits::Iterator       Iterator;
    typedef typename Traits::ConstIterator  ConstIterator;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
protected:
    virtual void _V_increment_iterator( IteratorState & it, size_t n ) const override {
            it += n; }
    virtual void _V_decrement_iterator( IteratorState & it, size_t n ) const override {
            it -= n;}
    virtual bool _V_compare_iterators( const size_t plhs, const size_t prhs ) const override {
                return plhs == prhs;
            }
    virtual Data & _V_dereference_iterator_state( IteratorState & it ) override {
                return const_cast<size_t&>(it);
            }
    virtual const Data & _V_dereference_iterator_state( const IteratorState & it ) const override {
                return const_cast<size_t&>(it);
            }
public:
    Iterator begin() {             return      Iterator(this, 0); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
};

// ++, +=

class IterableConsumerOnedirFwd : public hph::IterableTraits<
            size_t,
            size_t,
            void,
            hph::mixins::RandomForwardIterable
        >::Base {
public:
    typedef hph::IterableTraits<
                size_t,
                size_t,
                void,
                hph::mixins::RandomForwardIterable>     Traits;
    typedef typename Traits::Iterator       Iterator;
    typedef typename Traits::ConstIterator  ConstIterator;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
protected:
    virtual void _V_increment_iterator( IteratorState & it, size_t n ) const override {
            it += n; }
    virtual bool _V_compare_iterators( const size_t plhs, const size_t prhs ) const override {
                return plhs == prhs;
            }
    virtual Data & _V_dereference_iterator_state( IteratorState & it ) override {
                return const_cast<size_t&>(it);
            }
    virtual const Data & _V_dereference_iterator_state( const IteratorState & it ) const override {
                return const_cast<size_t&>(it);
            }
public:
    Iterator begin() {             return      Iterator(this, 0); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
};

// --, -=

class IterableConsumerOnedirBwd : public hph::IterableTraits<
            size_t,
            size_t,
            void,
            hph::mixins::RandomBackwardIterable
        >::Base {
public:
    typedef hph::IterableTraits<
                size_t,
                size_t,
                void,
                hph::mixins::RandomBackwardIterable>     Traits;
    typedef typename Traits::Iterator       Iterator;
    typedef typename Traits::ConstIterator  ConstIterator;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
protected:
    virtual void _V_decrement_iterator( IteratorState & it, size_t n ) const override {
            it += n; }
    virtual bool _V_compare_iterators( const size_t plhs, const size_t prhs ) const override {
                return plhs == prhs;
            }
    virtual Data & _V_dereference_iterator_state( IteratorState & it ) override {
                return const_cast<size_t&>(it);
            }
    virtual const Data & _V_dereference_iterator_state( const IteratorState & it ) const override {
                return const_cast<size_t&>(it);
            }
public:
    Iterator begin() {             return      Iterator(this, 0); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
};

// ++, --

class IterableConsumerBidirSerial : public hph::IterableTraits<
            size_t,
            size_t,
            void,
            hph::mixins::ForwardIterable,
            hph::mixins::BackwardIterable
        >::Base {
public:
    typedef hph::IterableTraits<
                size_t,
                size_t,
                void,
                hph::mixins::ForwardIterable,
                hph::mixins::BackwardIterable>     Traits;
    typedef typename Traits::Iterator       Iterator;
    typedef typename Traits::ConstIterator  ConstIterator;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
protected:
    virtual void _V_increment_iterator( IteratorState & it ) const override {
            ++it; }
    virtual void _V_decrement_iterator( IteratorState & it ) const override {
            --it;}
    virtual bool _V_compare_iterators( const size_t plhs, const size_t prhs ) const override {
                return plhs == prhs;
            }
    virtual Data & _V_dereference_iterator_state( IteratorState & it ) override {
                return const_cast<size_t&>(it);
            }
    virtual const Data & _V_dereference_iterator_state( const IteratorState & it ) const override {
                return const_cast<size_t&>(it);
            }
public:
    Iterator begin() {             return      Iterator(this, 0); }
    ConstIterator cbegin() const { return ConstIterator(this, 0); }
};

//
// Testing code
//

HPH_UT_BGN( Iterators )

{
    IterableConsumerBidirSerial bidirTesterSerial1,
                                bidirTesterSerial2;
    {
        auto it1 = bidirTesterSerial1.begin();
        auto it2 = bidirTesterSerial1.cbegin();
        auto it3 = bidirTesterSerial2.cbegin();
        auto it4 = bidirTesterSerial2.begin();
        // Just compile check: comparison of const/non-const iterators:
        bool testingTrueComparisons[] = {
                it1 == it1,
                it1 != it3,
                it4 != it1,
                it2 == it1,
                it3 == it4,
                it4 != it2,
                // ... whatever
            };
        for( uint8_t i = 0; i < sizeof(testingTrueComparisons); ++i ) {
            _ASSERT( testingTrueComparisons[i], "Comparison %d #1.", (int) i )
        } 
        it2++;
        auto it5(it1);
        _ASSERT( it5 == it1, "Copy constructor malfunction." );
        for( uint8_t i = 0; i < 8; ++i ) {
            it1++;
        }
        for( uint8_t i = 0; i < 7; ++i ) {
            it1--;
        }
        _ASSERT( it1 == it2, "Iterators comparison failure 1.1: %zd != %zd", *it1, *it2 );
    }
}
{
    IterableConsumerOnedirFwd onedirTesterFwd;
    {
        auto it = onedirTesterFwd.cbegin();
        it += 10;
        _ASSERT( 10 == *it, "Iterators comparison failure #2.1." );
        ++it;
        _ASSERT( 11 == *it, "Iterators comparison failure #2.2." );
    }
    {
        auto it = onedirTesterFwd.begin();
        auto it2 = it + 10;
        _ASSERT( 10 == *it2, "Iterators comparison failure #3.1: %zd.", *it2 );
    }
    IterableConsumerOnedirBwd onedirTesterBwd;
    {
        auto it = onedirTesterBwd.cbegin();
        it -= 7;
        _ASSERT(  7 == *it, "Iterators comparison failure #4.1." );
        --it;
        _ASSERT(  8 == *it, "Iterators comparison failure #4.2." );
    }
    {
        auto it = onedirTesterBwd.begin();
        auto it2 = it - 10;
        _ASSERT( 10 == *it2, "Iterators comparison failure #5.1: %zd.", *it2 );
    }
}
{
    IterableConsumerBidir bidirTester;
    {
        auto it = bidirTester.cbegin();
        it += 10;
        _ASSERT( 10 == *it, "Iterators comparison failure #6.1" );
        it++;
        _ASSERT( 11 == *it, "Iterators comparison failure #6.2." );
        it -= 7;
        _ASSERT(  4 == *it, "Iterators comparison failure #6.3." );
        --it;
        _ASSERT(  3 == *it, "Iterators comparison failure #6.4." );
    }
    {
        auto it1 = bidirTester.begin();
        auto it2 = bidirTester.cbegin();
        // Just compile check: comparison of const/non-const iterators:
        bool testingTrueComparisons[] = {
                it1 == it1,
                it2 == it1,
                0 == std::distance<IterableConsumerBidir::ConstIterator>( it1, it2 ),
                0 == std::distance( bidirTester.begin() + 1, ++it1 )
            };
        for( uint8_t i = 0; i < sizeof(testingTrueComparisons); ++i ) {
            _ASSERT( testingTrueComparisons[i], "Comparison %d #2.", (int) i )
        } 
    }
    {
        auto it = bidirTester.begin();
        IterableConsumerBidir::ConstIterator it2 = it + 10;
        size_t delta = std::distance<IterableConsumerBidir::ConstIterator>( it, it2 );
        _ASSERT( 10 == delta,
                 "Difference operator returns a wrong value: %zd != %zd",
                 (size_t) 10, delta );
    }
}


HPH_UT_END( Iterators )

