# ifndef GOO_XFORM_TCC
# define GOO_XFORM_TCC

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

# include "goo_utility.hpp"
# include "goo_exception.hpp"
# include <list>
# include <cassert>

/**@file xform.tcc
 * @brief An X-Form and automatically adjustable numerical ranges templates.
 *
 * Header file containing template routines for manipulating rectangular
 * numerical ranges.
 *
 * @todo: Despite of its name, the X-Form class by itself is still not
 * presented here. The X-Form is an abstraction of N-ary rectangular
 * region that changes its shape in such a way that re-scaling it along
 * one of the dimension causes immediate scaling along all the others
 * keeping the ratio between all the sides (thus, keeping also the angle
 * between diagonals --- here the "X" in its name comes from).
 */

namespace goo {
namespace aux {

/// Base class for numerical range.
struct AbstractRange {
    enum struct LimitIndex : uint8_t {
        both = 0,
        min = 1,
        max = 2,
    };
};

/**@class Range
 * @brief A numerical range representation.
 *
 * Auxiliary class implementing numerical range with
 * two limits --- upper and lower (minimum, maximum).
 * The point is to provide interfacing functions for
 * further classes like aux::XForm.
 */
template<typename NumTypeT>
class iRange : public AbstractRange {
public:
    typedef NumTypeT NumType;
    using AbstractRange::LimitIndex;
protected:
    union {
        NumType byIndex[2];
        struct {
            NumType lower, upper;
        } byName;
    } _ranges;

    /// Sets lower limit to provided value.
    virtual void _V_set_lower_limit_to( NumType nv )
        { _ranges.byName.lower = nv; }

    /// Sets upper limit to provided value.
    virtual void _V_set_upper_limit_to( NumType nv )
        { _ranges.byName.upper = nv; }

    /// Returns whether lower limit is uninitialized.
    virtual bool _V_min_uninitialized( const NumType & ) const = 0;

    /// Returns whether upper limit is uninitialized.
    virtual bool _V_max_uninitialized( const NumType & ) const = 0;
private:
    iRange() = delete;
protected:
    /// Ctr for subclasses.
    iRange( const NumType lower_, const NumType upper_ ) : _ranges{ lower_, upper_ } { }
public:

    /// Copy ctr. Wraps memcpy().
    iRange( const iRange<NumType> & );

    /// Dft dtr.
    virtual ~iRange() {}

    /// Getter for minimal value.
    const NumType lower() const {
        assert( !_V_min_uninitialized( _ranges.byName.lower ) );
        return _ranges.byName.lower;
    }

    /// Setter for minimal value.
    void lower( NumType nv )
        { _V_set_lower_limit_to( nv ); }

    /// Getter for maximal value.
    const NumType upper() const {
        assert( !_V_max_uninitialized( _ranges.byName.upper ) );
        return _ranges.byName.upper;
    }

    /// Setter for maximal value.
    void upper( NumType nv )
        { _V_set_upper_limit_to( nv ); }

    /// Directly returns const ptr to ranges data.
    /// No check performed.
    const NumType * ranges_data_ptr() const { return _ranges.byIndex; }

    /// Returns whether range(s) is (are) uninitialized.
    bool is_uninitialized( LimitIndex li ) const {
        if( !((uint8_t) li) ) {
            return _V_min_uninitialized( _ranges.byName.lower )
                || _V_max_uninitialized( _ranges.byName.upper );
        } else if ( LimitIndex::min == li ) {
            return _V_min_uninitialized( _ranges.byName.lower );
        } else {
            return _V_max_uninitialized( _ranges.byName.upper );
        }
    }

    /// Returns whether range(s) is (are) initialized.
    bool is_initialized( LimitIndex li ) const {
        return !is_uninitialized( LimitIndex::both );
    }

    /// Immediately sets upper limit to argument value if argument
    /// is greater than upper limit or upper limit is uninitialized.
    /// \arg val numeric value to be considered.
    /// \returns true, if upper limit was updated.
    /// \note If upper limit was uninitialized, updating will
    /// be also performed and true will be returned.
    virtual bool extend_if_above( const NumType & val ) {
        if( val > _ranges.byName.upper
            || _V_max_uninitialized( _ranges.byName.upper ) ) {
            _ranges.byName.upper = val;
            return true;
        }
        return false;
    }

    /// Immediately sets lower limit to argument value if argument
    /// is lesser than lower limit or lower limit is uninitialized.
    /// \arg val numeric value to be considered.
    /// \returns true, if lower limit was updated.
    /// \note If lower limit was uninitialized, updating will
    /// be also performed and true will be returned.
    virtual bool extend_if_below( const NumType & val ) {
        if( val < _ranges.byName.lower
            || _V_min_uninitialized( _ranges.byName.lower ) ) {
            _ranges.byName.lower = val;
            return true;
        }
        return false;
    }

    /// If provided value is greater than upper limit or lesser than
    /// lower limit, the corresponding limit will be set.
    /// \note If limit(s) are uninitialized, they will be also updated
    /// and set to provided
    /// value.
    /// \arg val value to be considered.
    /// \returns whether limit(s) was (were) updated.
    virtual bool extend_to( const NumType & val ) {
        if( is_uninitialized( LimitIndex::both ) ) {
            bool wasUpdated  = extend_if_above( val );
                 wasUpdated |= extend_if_below( val );
            return wasUpdated;
        }
        return extend_if_above( val ) || extend_if_below( val );
    }

    /// Returns true, if both ranges are set.
    virtual bool limits_valid() const {
        return !(_V_min_uninitialized( _ranges.byName.lower )
              || _V_max_uninitialized( _ranges.byName.upper ));
    }
};  // class Range


/**@class WidthCachedRange
 * @brief An iRange implementation caching its width.
 * */
template<typename NumTypeT>
class WidthCachedRange : public iRange<NumTypeT> {
public:
    typedef iRange<NumTypeT> Parent;
    typedef typename Parent::NumType NumType;
private:
    /// Width cache.
    mutable NumType _width;
    /// Is width cache valid?
    mutable bool _isWidthValid;
    WidthCachedRange() = delete;
protected:
    /// Performs re-computation of range width.
    virtual void _recache_width() const
        { assert( this->limits_valid() );
          _width = Parent::upper() - Parent::lower();
          _isWidthValid = true; }
    /// Invalidates width cache.
    virtual void _invalidate_width_cache()
        { _isWidthValid = false; }
    WidthCachedRange( NumType min, NumType max ) : _isWidthValid(false) {}

    /// Invalidate cache and set value according to parent method.
    virtual void _V_set_lower_limit_to( NumType nv ) override
        { _invalidate_width_cache(); _V_set_lower_limit_to(nv); }

    /// Invalidate cache and set value according to parent method.
    virtual void _V_set_upper_limit_to( NumType nv ) override
        { _invalidate_width_cache(); _V_set_upper_limit_to(nv); }

    bool is_width_valid() const { return _isWidthValid; }  
public:
    NumType range_width() const {
        if( !is_width_valid() ) { _recache_width(); }
            return _width;
    }
};  // class Range


/**@brief Numeric range implementation with cached width.
 *
 * Generic template is unimplemented. Currently the supported
 * cases include only the floating-point numbers (since they
 * are most often being required by applications). The floating
 * point range supports normalization procedure when input number
 * that lies in range should be linearilly projected inside [0:1]
 * range (this called "mapping"). De-normalization, when argument
 * that lies inside numerical range [0:1] is projected back is
 * called here reverse mapping.
 */
template<class FloatT, class Enable=void>
class Range; // default undefined
 
template<class FloatT>
class Range<FloatT, typename std::enable_if<std::is_floating_point<FloatT>::value >::type> :
            public WidthCachedRange<FloatT> {
public:
    typedef WidthCachedRange<FloatT> Parent;
    typedef typename Parent::NumType Float;
private:
    mutable FloatT _scale;
protected:
    virtual bool _V_min_uninitialized( const Float & val ) const override
            { return std::isnan( val ); }
    virtual bool _V_max_uninitialized( const Float & val ) const override
            { return std::isnan( val ); }
    /// Recalculates scale value = 1/width.
    virtual void _recache_width() const override
        { Parent::_recache_width(); _scale = 1/this->range_width(); }
public:
    /// Returns scale; will re-compute it if required.
    FloatT scale() const { if( !Parent::is_width_valid() ) { _recache_width(); } return _scale; }

    /// Defautl ctr.
    Range() : Parent() {}

    /// Ctr with ranges. Does not perform immediate re-calculation for given ranges.
    Range( FloatT min, FloatT max ) : Parent( min, max ) {}

    /// Performs normalization of provided value to [0:1].
    FloatT norm( FloatT unnormed ) const {
        if( this->limits_valid() ) {
            emraise( badState,
                     "Limits are invalid/unset for range %p : [%e, %e].",
                     this, this->lower(), this->upper() );
        }
        return (unnormed - this->lower())*scale();
    }

    /// Perform de-normalization of provided value from [0:1].
    FloatT denorm( FloatT normed ) const {
        assert( normed >= 0. && normed <= 1. );
        if( this->limits_valid() ) {
            emraise( badState,
                     "Limits are invalid/unset for range %p : [%e, %e].",
                     this, this->lower(), this->upper());
        }
        return normed*(this->range_width()) + (this->lower());
    }

    /// Returns true, if both ranges are set and finite.
    virtual bool limits_valid() const {
        return Parent::limits_valid()
            && std::isfinite(this->lower())
            && std::isfinite(this->upper());
    }
};  // class Range (floating point numbers)

// TODO: class Range (integral numbers)
# if 0
template<class FloatT>
class Range<FloatT, typename std::enable_if<std::is_integral<FloatT>::value >::type> :
            public WidthCachedRange<FloatT> {
public:
    typedef WidthCachedRange<FloatT> Parent;
    typedef typename Parent::NumType Float;
private:
    bool _minValid,
         _maxValid;
protected:
    virtual bool _V_min_uninitialized( const Float & ) const override
            { return _minValid; }
    virtual bool _V_max_uninitialized( const Float & ) const override
            { return _maxValid; }
    Range() : WidthCachedRange<FloatT>( 0, 0), _minValid(false), _maxValid(false) {}
};  // class Range (integral numbers)
# endif


/**@brief Multivariate region class for normalization.
 * @class NormedMultivariateRange
 *
 * Implements N-ary rectangular region mainly designed to fit
 * N-ary points (being extended to them). Aggregates N numerical
 * ranges that consequently adjusted to fit the point. (De)normalization
 * coefficients are computed by demand.
 * */
template<typename FloatT, uint8_t DT>
class NormedMultivariateRange {
public:
    static_assert( DT > 0, "NormedMultivariateRange can not be declared for 0 demensions." );
    constexpr static uint8_t D = DT;
    typedef FloatT Float;
    typedef NormedMultivariateRange<Float, D> Self;
protected:
    std::array<Range<Float> *, D> _rangesPtrs;
    virtual bool _V_extend_to( const Float * x ) {
        const Float * cx = x;
        bool updated = false;
        for( auto rangePtr : _rangesPtrs ) {
            updated |= rangePtr->extend_to( *cx );
            cx++;
        }
        return updated;
    }
public:
    /// Considers given vector as representative one for adjusting norming regions.
    bool extend_to( const Float * x ) {
        return _V_extend_to(x);
    }

    /// Multivariate array updating shortcut.
    template<typename T>
    bool extend_to( const std::array<
        typename std::enable_if<std::is_floating_point<T>::value, T>::type
        , D> & vars ) {
        std::array<Float, D> arrCopy;
        std::copy(std::begin(vars), std::end(vars), std::begin(arrCopy));
        return update_ranges( arrCopy.data() );
    }

    bool extend_to( const std::array<Float, D> & vars ) {
        /// No copying needed.
        return update_ranges( vars.data() );
    }

    /// Sets all ranges at once.
    virtual void set_ranges( const Float * xMins, const Float * xMaxs ) {
        uint8_t d = 0;
        for( auto rangePtr : _rangesPtrs ) {
            if( !(std::isfinite(xMins[d]) && std::isfinite(xMaxs[d]))
                    || xMins[d] >= xMaxs[d] ) {
                emraise( badValue, "XForm ranges for dimension #%d couldn't be set to [%e, %e] as "
                         "min >= max or one of bounds is not finite.",
                         (int) d, xMins[d], xMaxs[d] );
            }
            rangePtr->lower( xMins[d] );
            rangePtr->upper( xMaxs[d] );
            ++d;
        }
    }

    /// Returns true, when ranges are were set.
    bool ranges_set() const {
        bool allValid = true;
        for( auto & rangePtr : _rangesPtrs ) {
            allValid &= rangePtr->limits_valid();
        }
        return allValid;
    }

    /// Const getter for range:
    const Range<Float> & range( uint8_t d ) const {
        assert( d < D );
        return *(_rangesPtrs[d]);
    }

    /// Performs no-copy normalization (forward mapping).
    virtual void norm( Float * x ) const {
        uint8_t d = 0;
        for( auto rangePtr : _rangesPtrs ) {
            x[d] = rangePtr->norm( x[d] );
            ++d;
        }
    }

    /// Preforms denormalization of normalized value (reverse mapping).
    virtual void denorm( Float * x ) const {
        uint8_t d = 0;
        for( auto rangePtr : _rangesPtrs ) {
            x[d] = rangePtr->denorm( x[d] );
            ++d;
        }
    }

    /// Checks, if given vector fits the expected range.
    virtual bool match( Float * x ) const {
        uint8_t d = 0;
        for( auto & rangePtr : _rangesPtrs ) {
            if( rangePtr[d].lower() > x[d]
             || rangePtr[d].upper() < x[d] ) {
                return false;
            }
            ++d;
        }
        return true;
    }

    template<typename DestNumT>
    void extend_with( const NormedMultivariateRange<DestNumT, D> & o ) {
        for( uint8_t d = 0; d < D; ++d ) {
            _rangesPtrs[d]->extend_to( o.range(d).lower() );
            _rangesPtrs[d]->extend_to( o.range(d).upper() );
        }
    }
};  // class NRange

}  // namespace aux
}  // namespace goo

# endif  // GOO_XFORM_TCC

