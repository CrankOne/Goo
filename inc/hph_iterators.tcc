# ifndef H_HPH_ITERABLE_MIXIN_H
# define H_HPH_ITERABLE_MIXIN_H

# include "hph_types.h"
# include "hph_utility.hpp"

namespace hph {
namespace mixins {

//
// Iterable mixin
//

/**@brief Iterable base template class.
 *
 * Declares some common interface features for iterable entities.
 * Is not intended for direct use.
 */
template<typename IterationTraitsP>
class IterableBase {
public:
    typedef IterationTraitsP                Traits;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;

    class _ConstIterator {
    private:
        const IterableBase * _target;  // const
    protected:
        IteratorState  _state;
    public:
        _ConstIterator() : _target(nullptr) {}
        template<typename ... StateCtrArgs>
        _ConstIterator(const IterableBase * target, StateCtrArgs ... ctrArgs) : _target(target), _state(ctrArgs...){}

        const IterableBase * target() const { return _target; };
        void target( const IterableBase * t ) { _target = t; };

        const IteratorState & state() const { return _state; }
        IteratorState state_copy() const { return _state; }
        virtual ~_ConstIterator(){}
        const Data & operator*() const {
            return _target->_V_dereference_iterator_state(_state); }

        bool operator==(_ConstIterator const& prhs) {
            return _target != prhs._target ? false : _target->_V_compare_iterators( _state, prhs.state() );
        }

        bool operator!=(_ConstIterator const& prhs) {
            return _target != prhs._target ? true : !_target->_V_compare_iterators( _state, prhs.state() );
        }
    };

    class _Iterator {
    private:
        IterableBase * _target;  // const
    protected:
        IteratorState  _state;
    public:
        _Iterator() : _target(nullptr) {}
        template<typename ... StateCtrArgs>
        _Iterator(IterableBase * target, StateCtrArgs ... ctrArgs) : _target(target), _state(ctrArgs...){}
        virtual ~_Iterator(){}

        IterableBase * target() { return _target; };
        const IterableBase * target() const { return _target; };
        void target( IterableBase * t ) { _target = t; };

        const IteratorState & state() const { return _state; }
        IteratorState state_copy() const { return _state; }
        
        Data & operator*() {
            return _target->_V_dereference_iterator_state(_state); }

        operator _ConstIterator() { return { _target, _state }; }

        bool operator==(_ConstIterator const& prhs) {
            return _target != prhs.target() ? false : _target->_V_compare_iterators( _state, prhs.state() );
        }

        bool operator!=(_ConstIterator const& prhs) {
            return _target != prhs.target() ? true : !_target->_V_compare_iterators( _state, prhs.state() );
        }
    };
protected:
    /// Provides actual dereferencing for given iterator's state.
    virtual Data & _V_dereference_iterator_state( IteratorState & ) = 0;
    /// (const ver.) Provides actual dereferencing for given iterator's state.
    virtual const Data & _V_dereference_iterator_state( const IteratorState & ) const = 0;
    /// Compares iterator states.
    virtual bool _V_compare_iterators( const IteratorState plhs, const IteratorState prhs ) const = 0;
public:
    virtual ~IterableBase(){}
};

// FWD

template<typename IterationTraitsP>
struct ForwardIterable : public virtual IterableBase<IterationTraitsP> {
public:
    typedef IterationTraitsP                Traits;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
    typedef IterableBase<Traits>            Parent;
    typedef ForwardIterable<Traits>         Self;

    struct _Iterator : public virtual Parent::_Iterator {
    public:
        // STL compliance
        typedef typename Traits::difference_type difference_type;
        typedef typename Traits::value_type      value_type;
        typedef typename Traits::pointer         pointer;
        typedef typename Traits::reference       reference;
    private:
        Self * _target;
    public:
        _Iterator( Self * t ) :
            Parent::_Iterator( static_cast<Parent*>(t)),
            _target(t) {}
        _Iterator() :
            Parent::_Iterator(),
            _target(nullptr) {}

        Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( Self * t ) { _target = t; Parent::_Iterator::target(t); }

        typename Traits::Iterator & operator++() {
            _target->increment_iterator(this->_state);
            return static_cast<typename Traits::Iterator&>(*this);
        }
        typename Traits::Iterator operator++(int) {
            typename Traits::Iterator copy(*static_cast<typename Traits::Iterator*>(this));
            _target->increment_iterator(this->_state);
            return copy;
        }
    };

    struct _ConstIterator : public virtual Parent::_ConstIterator {
    private:
        const Self * _target;
    public:
        _ConstIterator( const Self * t ) :
            Parent::_ConstIterator( static_cast<const Parent*>(t)),
            _target(t) {}
        _ConstIterator() :
            Parent::_ConstIterator(),
            _target(nullptr) {}
        const Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( const Self * t ) { _target = t; Parent::_ConstIterator::target(t); }

        typename Traits::ConstIterator & operator++() {
            _target->increment_iterator(this->_state);
            return static_cast<typename Traits::ConstIterator&>(*this);
        }
        typename Traits::ConstIterator operator++(int) {
            typename Traits::ConstIterator copy(*static_cast<const typename Traits::ConstIterator*>(this));
            _target->increment_iterator(this->_state);
            return copy;
        }
    };
protected:
    virtual void _V_increment_iterator( IteratorState & ) const = 0;
public:
    void increment_iterator( IteratorState & it ) const {
        _V_increment_iterator( it ); }
};

template<typename IterationTraitsP>
struct RandomForwardIterable :
        public ForwardIterable<IterationTraitsP> {
public:
    typedef IterationTraitsP                Traits;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
    typedef ForwardIterable<Traits>         Parent;
    typedef RandomForwardIterable<Traits>   Self;

    struct _Iterator : public Parent::_Iterator {
    public:
        // STL compliance
        typedef typename Traits::difference_type difference_type;
        typedef typename Traits::value_type      value_type;
        typedef typename Traits::pointer         pointer;
        typedef typename Traits::reference       reference;
    private:
        Self * _target;
    public:
        _Iterator( Self * t ) :
            Parent::_Iterator( static_cast<Parent*>(t)),
            _target(t) {}
        _Iterator() :
            Parent::_Iterator(),
            _target(nullptr) {}

        Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( Self * t ) { _target = t; Parent::_Iterator::target(t); }

        typename Traits::Iterator & operator+=(size_t n) {
            _target->increment_iterator(this->_state, n);
            return static_cast<typename Traits::Iterator&>(*this);
        }

        template<typename IteratorTP>
        friend IteratorTP operator+( const IteratorTP & it, size_t offset );
    };

    struct _ConstIterator : public Parent::_ConstIterator {
    private:
        const Self * _target;
    public:
        _ConstIterator( const Self * t ) :
            Parent::_ConstIterator( static_cast<const Parent*>(t)),
            _target(t) {}
        _ConstIterator() :
            Parent::_ConstIterator(),
            _target(nullptr) {}
        const Self * target() const { return _target; }
        void target( const Self * t ) { _target = t; Parent::_ConstIterator::target(t); }

        typename Traits::ConstIterator & operator+=(size_t n) {
            _target->increment_iterator(this->_state, n);
            return static_cast<typename Traits::ConstIterator&>(*this);
        }
    };
protected:
    virtual void _V_increment_iterator( IteratorState &, size_t ) const = 0;
    virtual void _V_increment_iterator( IteratorState & it ) const override {
        _V_increment_iterator(it, 1 ); }
public:
    void increment_iterator( IteratorState & it, size_t n ) const {
        _V_increment_iterator(it, n); }
};

template<typename IteratorTP> // TODO: restriction
IteratorTP operator+( const IteratorTP & it, size_t offset ) {
    return IteratorTP(it) += offset;
}

// BWD

template<typename IterationTraitsP>
struct BackwardIterable : public virtual IterableBase<
        IterationTraitsP> {
public:
    typedef IterationTraitsP                Traits;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
    typedef IterableBase<Traits>            Parent;
    typedef BackwardIterable<Traits>        Self;

    struct _Iterator : public virtual Parent::_Iterator {
    private:
        Self * _target;
    public:
        _Iterator( Self * t ) :
            Parent::_Iterator( static_cast<Parent*>(t)),
            _target(t) {}
        _Iterator() :
            Parent::_Iterator(),
            _target(nullptr) {}
        Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( Self * t ) { _target = t; Parent::Iterator::target(t); }

        typename Traits::Iterator & operator--() {
            _target->decrement_iterator(this->_state);
            return static_cast<typename Traits::Iterator&>(*this);
        }
        typename Traits::Iterator operator--(int) {
            typename Traits::Iterator copy(*static_cast<typename Traits::Iterator*>(this));
            _target->decrement_iterator(this->_state);
            return copy;
        }
    };

    struct _ConstIterator : public virtual Parent::_ConstIterator {
    private:
        const Self * _target;
    public:
        _ConstIterator( const Self * t ) :
            Parent::_ConstIterator( static_cast<const Parent*>(t)),
            _target(t) {}
        _ConstIterator() :
            Parent::_ConstIterator(),
            _target(nullptr) {}
        const Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( const Self * t ) { _target = t; Parent::_ConstIterator::target(t); }

        typename Traits::ConstIterator & operator--() {
            _target->decrement_iterator(this->_state);
            return static_cast<typename Traits::ConstIterator&>(*this);
        }
        typename Traits::ConstIterator operator--(int) {
            typename Traits::ConstIterator copy(*static_cast<const typename Traits::ConstIterator*>(this));
            _target->decrement_iterator(this->_state);
            return copy;
        }
    };
protected:
    virtual void _V_decrement_iterator( IteratorState & ) const = 0;
public:
    void decrement_iterator( IteratorState & it ) const {
        _V_decrement_iterator( it ); }
};

template<typename IterationTraitsP>
struct RandomBackwardIterable :
        public BackwardIterable<IterationTraitsP> {
public:
    typedef IterationTraitsP                Traits;
    typedef typename Traits::Data           Data;
    typedef typename Traits::IteratorState  IteratorState;
    typedef BackwardIterable<Traits>        Parent;
    typedef RandomBackwardIterable<Traits>  Self;

    struct _Iterator : public Parent::_Iterator {
    private:
        Self * _target;
    public:
        _Iterator( Self * t ) :
            Parent::_Iterator( static_cast<Parent*>(t)),
            _target(t) {}
        _Iterator() :
            Parent::_Iterator(),
            _target(nullptr) {}
        Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( Self * t ) { _target = t; Parent::Iterator::target(t); }

        typename Traits::Iterator & operator-=(size_t n) {
            _target->decrement_iterator(this->_state, n);
            return static_cast<typename Traits::Iterator&>(*this);
        }
    };

    struct _ConstIterator : public Parent::_ConstIterator {
    private:
        const Self * _target;
    public:
        _ConstIterator( const Self * t ) :
            Parent::_ConstIterator( static_cast<const Parent*>(t)),
            _target(t) {}
        _ConstIterator() :
            Parent::_ConstIterator(),
            _target(nullptr) {}
        const Self * target() { return _target; }
        const Self * target() const { return _target; }
        void target( const Self * t ) { _target = t; Parent::_ConstIterator::target(t); }

        typename Traits::ConstIterator & operator-=(size_t n) {
            _target->decrement_iterator(this->_state, n);
            return static_cast<typename Traits::ConstIterator&>(*this);
        }
    };
protected:
    virtual void _V_decrement_iterator( IteratorState &, size_t ) const = 0;
    virtual void _V_decrement_iterator( IteratorState & it ) const override {
        _V_decrement_iterator(it, 1); }
public:
    void decrement_iterator( IteratorState & it, size_t n ) const {
        _V_decrement_iterator(it, n); }
};

template<typename IteratorTP>  // TODO: restriction
IteratorTP operator-( const IteratorTP & it, size_t offset ) {
    return IteratorTP(it) -= offset;
}

}  // namespace mixins

template<typename DereferencedDataTP,
         typename IteratorStateTP,
         typename DistanceTP,
         template<class> class ... IterableBases>
struct IterableTraits {
    typedef DereferencedDataTP  Data;
    typedef IteratorStateTP     IteratorState;
    typedef IterableTraits<Data, IteratorState, DistanceTP, IterableBases ...> Self;
    // STL compliance
    typedef DistanceTP  difference_type;
    typedef Data        value_type;
    typedef Data*       pointer;
    typedef Data&       reference;

    class Base : public IterableBases<Self> ... { };
    class Iterator;
    class ConstIterator;

    static_assert(   (std::is_base_of<mixins::RandomForwardIterable<Self>,  Base>::value &&
                      std::is_base_of<mixins::RandomBackwardIterable<Self>, Base>::value) ^
                      std::is_void<difference_type>::value,
                "For iterators that does not provide random access for both directions, difference type must be void and non-void for others.");

    struct ConstIterator : public IterableBases<Self>::_ConstIterator ... {
        typedef DereferencedDataTP Data;
        typedef IteratorStateTP    IteratorState;
        typedef Self               Traits;

        template<typename ... StateCtrArgs>
        ConstIterator( const Base * b, StateCtrArgs ... stateCtrArgs ) :
                mixins::IterableBase<Self>::_ConstIterator(b, stateCtrArgs ...),
                IterableBases<Self>::_ConstIterator(b) ... {}
        ConstIterator( const Iterator & o ) :
                mixins::IterableBase<Self>::_ConstIterator(static_cast<const typename mixins::IterableBase<Self>::_Iterator&>(o).target(), o.state_copy()),
                IterableBases<Self>::_ConstIterator(static_cast<const typename IterableBases<Self>::_Iterator&>(o).target()) ... {}
        ConstIterator( ) :
                mixins::IterableBase<Self>::_ConstIterator(),
                IterableBases<Self>::_ConstIterator() ... {}

        // STL compliance
        typedef typename std::conditional<
                std::is_base_of<mixins::RandomForwardIterable<Self>,  Base>::value &&
                std::is_base_of<mixins::RandomBackwardIterable<Self>, Base>::value,
                        std::random_access_iterator_tag,
                        typename std::conditional<
                            std::is_base_of<mixins::ForwardIterable<Self>,  Base>::value &&
                            std::is_base_of<mixins::BackwardIterable<Self>, Base>::value,
                            std::bidirectional_iterator_tag,
                            typename std::conditional<
                                std::is_base_of<mixins::ForwardIterable<Self>,  Base>::value,
                                std::forward_iterator_tag,
                                std::input_iterator_tag
                            >::type
                        >::type
                    >::type iterator_category;

        typedef DistanceTP  difference_type;
        typedef const Data  value_type;
        typedef const Data* pointer;
        typedef const Data& reference;
    };

    struct Iterator : public IterableBases<Self>::_Iterator ... {
        typedef DereferencedDataTP Data;
        typedef IteratorStateTP    IteratorState;
        typedef Self               Traits;

        template<typename ... StateCtrArgs>
        Iterator( Base * b, StateCtrArgs ... stateCtrArgs ) :
                mixins::IterableBase<Self>::_Iterator(b, stateCtrArgs ...),
                IterableBases<Self>::_Iterator(b) ... {}
        Iterator( ) :
                mixins::IterableBase<Self>::_Iterator(),
                IterableBases<Self>::_Iterator() ... {}

        // STL compliance
        typedef typename std::conditional<
                std::is_base_of<mixins::RandomForwardIterable<Self>,  Base>::value &&
                std::is_base_of<mixins::RandomBackwardIterable<Self>, Base>::value,
                        std::random_access_iterator_tag,
                        typename std::conditional<
                            std::is_base_of<mixins::ForwardIterable<Self>,  Base>::value &&
                            std::is_base_of<mixins::BackwardIterable<Self>, Base>::value,
                            std::bidirectional_iterator_tag,
                            typename std::conditional<
                                std::is_base_of<mixins::ForwardIterable<Self>,  Base>::value,
                                std::forward_iterator_tag,
                                std::input_iterator_tag
                            >::type
                        >::type
                    >::type iterator_category;

        typedef DistanceTP  difference_type;
        typedef Data        value_type;
        typedef Data*       pointer;
        typedef Data&       reference;

        operator ConstIterator() { // XXX
            return ConstIterator(*this);
        }
    };
};

template< typename IteratorTP, typename std::enable_if<
        std::is_base_of<mixins::RandomForwardIterable<typename IteratorTP::Traits>,  typename IteratorTP::Traits::Base>::value &&
        std::is_base_of<mixins::RandomBackwardIterable<typename IteratorTP::Traits>, typename IteratorTP::Traits::Base>::value &&
        std::is_convertible<IteratorTP, typename IteratorTP::Traits::ConstIterator>::value
    >::type* = nullptr>
typename IteratorTP::Traits::difference_type operator-(IteratorTP const& plhs, IteratorTP const& prhs) {
    return plhs.state() - prhs.state();
}

}  // namespace hph

# endif  // H_HPH_ITERABLE_MIXIN_H

