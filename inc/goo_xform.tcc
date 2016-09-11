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
# include <ostream>

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
 *
 * @ingroup numRanges
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
 *
 * @ingroup numRanges
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
    /// Returns true, if limit set (useful for applications).
    virtual bool _V_set_lower_limit_to( NumType nv )
        { _ranges.byName.lower = nv; return true; }

    /// Sets upper limit to provided value.
    /// Returns true, if limit set (useful for applications).
    virtual bool _V_set_upper_limit_to( NumType nv )
        { _ranges.byName.upper = nv; return true; }

    /// Returns whether lower limit is uninitialized.
    virtual bool _V_min_uninitialized( const NumType ) const = 0;

    /// Returns whether upper limit is uninitialized.
    virtual bool _V_max_uninitialized( const NumType ) const = 0;
private:
    iRange() = delete;
protected:
    /// Ctr for subclasses.
    iRange( const NumType lower_, const NumType upper_ ) : _ranges{{lower_, upper_}} { }
public:

    /// Copy ctr. Wraps memcpy().
    iRange( const iRange<NumType> & );

    /// Dft dtr.
    virtual ~iRange() {}

    /// Getter for minimal value.
    const NumType lower() const {
        return _ranges.byName.lower;
    }

    /// Setter for minimal value.
    bool lower( NumType nv )
        { return _V_set_lower_limit_to( nv ); }

    /// Getter for maximal value.
    const NumType upper() const {
        return _ranges.byName.upper;
    }

    /// Setter for maximal value.
    bool upper( NumType nv )
        { return _V_set_upper_limit_to( nv ); }

    /// Directly returns const ptr to ranges data.
    /// No check performed.
    const NumType * ranges_data_ptr() const { return _ranges.byIndex; }

    /// Returns whether range(s) is (are) uninitialized.
    bool is_uninitialized( LimitIndex li=LimitIndex::both ) const {
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
    bool is_initialized( LimitIndex li=LimitIndex::both ) const {
        return !is_uninitialized( li );
    }

    /// Immediately sets upper limit to argument value if argument
    /// is greater than upper limit or upper limit is uninitialized.
    /// \arg val numeric value to be considered.
    /// \returns true, if upper limit was updated.
    /// \note If upper limit was uninitialized, updating will
    /// be also performed and true will be returned.
    virtual bool extend_if_above( const NumType val ) {
        if( val > _ranges.byName.upper
            || _V_max_uninitialized( _ranges.byName.upper ) ) {
            return upper( val );
        }
        return false;
    }

    /// Immediately sets lower limit to argument value if argument
    /// is lesser than lower limit or lower limit is uninitialized.
    /// \arg val numeric value to be considered.
    /// \returns true, if lower limit was updated.
    /// \note If lower limit was uninitialized, updating will
    /// be also performed and true will be returned.
    virtual bool extend_if_below( const NumType val ) {
        if( val < _ranges.byName.lower
            || _V_min_uninitialized( _ranges.byName.lower ) ) {
            return lower( val );
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
    virtual bool extend_to( const NumType val ) {
        if( is_uninitialized( LimitIndex::both ) ) {
            bool wasUpdated  = extend_if_above( val );
                 wasUpdated |= extend_if_below( val );
            return wasUpdated;
        }
        return extend_if_above( val ) || extend_if_below( val );
    }

    /// Returns true, if both ranges are set.
    virtual bool limits_valid() const {
        return !( _V_min_uninitialized( _ranges.byName.lower )
               || _V_max_uninitialized( _ranges.byName.upper )
               || _ranges.byName.lower == _ranges.byName.upper );
    }

    virtual bool is_in( const NumType val ) const {
        if( is_uninitialized() ) return false;
        return _ranges.byName.lower <= val
            && _ranges.byName.upper >= val
            ;
    }
};  // class Range


/**@class WidthCachedRange
 * @brief An iRange implementation caching its width.
 * @ingroup numRanges
 * */
template<typename NumTypeT>
class WidthCachedRange : public iRange<NumTypeT> {
public:
    typedef iRange<NumTypeT> Parent;
    typedef typename Parent::NumType NumType;
    using typename iRange<NumTypeT>::LimitIndex;
private:
    /// Width cache.
    mutable NumType _width;
    /// Is width cache valid?
    mutable bool _isWidthValid;
    WidthCachedRange() = delete;
protected:
    /// Performs re-computation of range width.
    virtual void _recache_width() const {
        if( ! this->limits_valid() ) {
            emraise(badState, "Recaching width invoked for invalid limits [%e, %e].",
                    this->lower(), this->upper());
        }
        _width = Parent::upper() - Parent::lower();
        if( _width < 0 ) {
            emraise( badState, "Negative width: %e < 0 according to limits [%e, %e].",
                    _width, this->lower(), this->upper() );
        }
        _isWidthValid = true;
    }
    /// Invalidates width cache.
    virtual void _invalidate_width_cache()
        { _isWidthValid = false; }
    WidthCachedRange( NumType min_, NumType max_ ) : iRange<NumTypeT>(min_, max_), _isWidthValid(false) {}

    /// Invalidate cache and set value according to parent method.
    virtual bool _V_set_lower_limit_to( NumType nv ) override {
        if( Parent::_V_set_lower_limit_to(nv) ) {
            _invalidate_width_cache();
            return true;
        }
        return false;
    }

    /// Invalidate cache and set value according to parent method.
    virtual bool _V_set_upper_limit_to( NumType nv ) override {
        if( Parent::_V_set_upper_limit_to(nv) ) {
            _invalidate_width_cache();
            return true;
        }
        return false;
    }

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
 *
 * @ingroup numRanges
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
    virtual bool _V_min_uninitialized( const Float val ) const override
            { return std::isnan( val ); }
    virtual bool _V_max_uninitialized( const Float val ) const override
            { return std::isnan( val ); }
    /// Recalculates scale value = 1/width.
    virtual void _recache_width() const override
        { Parent::_recache_width(); _scale = 1/this->range_width(); }
public:
    /// Returns scale; will re-compute it if required.
    FloatT scale() const { if( !Parent::is_width_valid() ) { _recache_width(); } return _scale; }

    /// Defautl ctr.
    Range() : Parent(
            std::numeric_limits<FloatT>::quiet_NaN(),
            std::numeric_limits<FloatT>::quiet_NaN()
        ) {}

    /// Ctr with ranges. Does not perform immediate re-calculation for given ranges.
    Range( FloatT min, FloatT max ) : Parent( min, max ) {}

    /// Performs normalization of provided value to [0:1].
    FloatT norm( FloatT unnormed ) const {
        if( !this->limits_valid() ) {
            emraise( badState,
                     "Limits are invalid/unset for range %p : [%e, %e].",
                     this, this->lower(), this->upper() );
        }
        return (unnormed - this->lower())*scale();
    }

    /// Perform de-normalization of provided value from [0:1].
    FloatT denorm( FloatT normed ) const {
        //assert( normed >= 0. && normed <= 1. );
        if( !this->limits_valid() ) {
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
 *
 * @ingroup numRanges
 * */
template<typename RangeT,
         uint8_t DT>
class NormedMultivariateRange {
public:
    static_assert( DT > 0, "NormedMultivariateRange can not be declared for 0 demensions." );
    constexpr static uint8_t D = DT;
    typedef RangeT RangeType;
    typedef typename RangeT::Float Float;
    typedef NormedMultivariateRange<Float, D> Self;
    static_assert( std::is_base_of<Range<Float>, RangeT>::value,
        "Range base type have to be inherited from Range<FloatT>." );
protected:
    std::array<RangeType *, D> _rangesPtrs;
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
    NormedMultivariateRange() {
        for( auto & rangePtr : _rangesPtrs ) {
            rangePtr = nullptr;
        }
    }

    /// Considers given vector as representative one for adjusting norming regions.
    bool extend_to( const Float * x ) {
        return _V_extend_to(x);
    }

    /// Multivariate array updating shortcut.
    template<typename T>
    bool extend_to( const std::array<
        typename std::enable_if<std::is_floating_point<T>::value, T>::type
        , D> & vars ) {
        assert( !_rangesPtrs.empty() );
        std::array<Float, D> arrCopy;
        std::copy(std::begin(vars), std::end(vars), std::begin(arrCopy));
        return update_ranges( arrCopy.data() );
    }

    bool extend_to( const std::array<Float, D> & vars ) {
        /// No copying needed.
        return update_ranges( vars.data() );
    }

    /// Sets all ranges at once.
    virtual void set_limits( const Float * xMins, const Float * xMaxs ) {
        uint8_t d = 0;
        for( auto rangePtr : _rangesPtrs ) {
            if( !(std::isfinite(xMins[d]) && std::isfinite(xMaxs[d]))
                    || xMins[d] >= xMaxs[d] ) {
                emraise( badValue, "XForm ranges for dimension #%d couldn't be set to [%e, %e] as "
                         "min >= max or one of bounds is not finite.",
                         (int) d, xMins[d], xMaxs[d] );
            }
            if( !rangePtr ) {
                emraise( badState,
                         "Range instance #%d isn't set for NormedMultivariateRange %p.",
                         (int) d, this );
            }
            rangePtr->lower( xMins[d] );
            rangePtr->upper( xMaxs[d] );
            ++d;
        }
    }

    /// Returns true, when ranges are were set.
    bool ranges_set() const {
        for( auto & rangePtr : _rangesPtrs ) {
            if( !(rangePtr && rangePtr->limits_valid()) ) {
                return false;
            }
        }
        return true;
    }

    /// Const getter for range:
    const Range<Float> & range( uint8_t d ) const {
        assert( d < D );
        assert( _rangesPtrs[d] );
        return *(_rangesPtrs[d]);
    }

    /// Mutable getter for range:
    Range<Float> & range( uint8_t d ) {
        assert( d < D );
        assert( _rangesPtrs[d] );
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
            if( !rangePtr[d].is_in(x[d]) ) {
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

    void set_range_instance( uint8_t nd, RangeType & rangeRef ) {
        assert( nd < D );
        _rangesPtrs[nd] = &rangeRef;
    }
};  // class NRange


/**@class iRangeAccumulator
 * @brief Interface for a set of dynamically re-computed numerical parameters.
 * @ingroup numRanges
 * */
template<typename FloatT,
         typename ToleranceParametersT>
class iRangeAccumulator {
protected:
    /// (iface) Returns true, if value was taken into consideration.
    virtual bool _V_accumulate( FloatT val ) = 0;
    /// (iface) Returns lower boundary for current state and provided tolerance parameters.
    virtual FloatT _V_boundary_lower( const ToleranceParametersT & ) const = 0;
    /// (iface) Returns upper boundary for current state and provided tolerance parameters.
    virtual FloatT _V_boundary_upper( const ToleranceParametersT & ) const = 0;
public:
    bool accumulate( FloatT val ) { return _V_accumulate( val ); }
    /// Returns lower boundary for current state and provided tolerance parameters.
    FloatT boundary_lower( const ToleranceParametersT & tp ) const
        { return _V_boundary_lower(tp); }
    /// Returns upper boundary for current state and provided tolerance parameters.
    FloatT boundary_upper( const ToleranceParametersT & tp ) const
        { return _V_boundary_upper(tp); }
    /// Returns true, if value lies inside of allowed numerical boundaries.
    virtual bool fits( FloatT val, const ToleranceParametersT & tp ) const
        { return val < boundary_upper(tp)
              && val > boundary_lower(tp); }
};  // class iRangeAccumulator

template<typename FloatT, typename ToleranceParametersT> class NormalAccumulator;

template<typename FloatT, typename ToleranceParametersT> std::ostream &
operator<<( std::ostream & os, const NormalAccumulator<FloatT, ToleranceParametersT> & acc );

/**@class NormalAccumulator
 * @brief Incremental algorithm for calculating unweighted real numbers
 * distribution parameters.
 *
 * This accumulator is designed for application where normal (Gaussian)
 * distribution is supposed for unweighted samples sets. Despite of that
 * it can yield a significant numerical error (due to rounding error) on
 * large sets, it still can be used in some applications where precision
 * is not a point of interest.
 *
 * Particular useful as a template parameter in classes like RigidRange
 * which provides adaptive numerical adjustements for online data.
 *
 * @ingroup numRanges
 * */
template<typename FloatT,
         typename ToleranceParametersT>
class NormalAccumulator : public iRangeAccumulator<FloatT, ToleranceParametersT> {
public:
    struct Cache {
        FloatT mean,
               squaredMean,
               rmsSquared,
               rms,
               variance,
               unbiasedVariance,
               sigma
               ;
        Cache() {
            mean = squaredMean = rmsSquared = rms =
                   variance = unbiasedVariance = sigma =
                   std::numeric_limits<FloatT>::quiet_NaN();
        }
    };
private:
    size_t _nEntries;
    FloatT _sum,
           _sumOfSquares
           ;

    mutable bool _isCacheValid;
    mutable Cache _cache;

    void _recache() const {
        if( !_nEntries ) {
            // TODO: if _nEntries < 2, set all to NaN? set caches to false?
            return;
        }
        _cache.mean             = _sum/_nEntries;
        _cache.squaredMean      = _cache.mean*_cache.mean;
        _cache.rmsSquared       = _sumOfSquares/_nEntries;
        _cache.rms              = sqrt(_sumOfSquares);
        const double commonVal = (_sumOfSquares - (_sum*_sum)/_nEntries);
        _cache.variance         = commonVal/_nEntries;
        _cache.unbiasedVariance = commonVal/(_nEntries-1);
        _cache.sigma            = sqrt( _cache.unbiasedVariance );
        // Cache now valid:
        _isCacheValid           = true;
    }
protected:
    /// Returns true, if value was taken into consideration.
    virtual bool _V_accumulate( FloatT val ) override {
        invalidate_cache();
        _sum += val;
        _sumOfSquares += val*val;
        ++_nEntries;
        return true;
    }
    /// Returns lower boundary for current state and provided tolerance parameters.
    virtual FloatT _V_boundary_lower( const ToleranceParametersT & tp ) const override {
        return cache().mean - 3*cache().sigma*tp;
    }
    /// Returns upper boundary for current state and provided tolerance parameters.
    virtual FloatT _V_boundary_upper( const ToleranceParametersT & tp ) const override {
        return cache().mean + 3*cache().sigma*tp;
    }
public:
    NormalAccumulator() : _nEntries(0),
                          _sum(0.),
                          _sumOfSquares(0.),
                          _isCacheValid(false) {}

    void invalidate_cache() { _isCacheValid = false; }

    /// Returns reference on cache instance. If number of considered entries is <2,
    /// the cache instance will contain only NaNs.
    const Cache cache() const {
        if( !_isCacheValid ) {
            _recache();
        }
        return _cache;
    }

    /// Returns true, if value can be taken intro consideration.
    virtual bool fits( FloatT val, const ToleranceParametersT & tp ) const override {
        if( _nEntries < 2 ) {
            // Too few samples considered --- fits all.
            return true;
        }
        return iRangeAccumulator<FloatT, ToleranceParametersT>::fits( val, tp );
    }

    // friend ostream operator?
};  // class NormalAccumulator

template<typename FloatT, typename ToleranceParametersT> std::ostream &
operator<<( std::ostream & os, const NormalAccumulator<FloatT, ToleranceParametersT> & acc ) {
    os << "{"
       << "mean : " << acc.cache().mean << ", "
       << "rms : " << acc.cache().rms << ", "
       << "sigma : " << acc.cache().sigma
       << "}";
    return os;
}

/**@brief Dynamic range gracifylly extending itself accordingly to
 * numerical data provided to it.
 *
 * This class implements lazy adjustment algorithm when values that
 * are much greater than averaged value won't be taken into
 * consideration.
 *
 * The adjustement is defined by a rigidness parameter
 * which is a rejection threshold. The value provided to extend_to()
 * method will be:
 *  - in case of upper limit extending, related to mean value and
 *    have to lie below the threshold ($A_{mean} / A_{new} < V_{thr}$)
 *    to be considered as new upper limit.
 *  - in case of lower limit extending, will be divided by mean value
 *    and to lie below the threshold ($A_{new}/A_{mean} < V_{thr}$)
 *    to be considered as new lower limit.
 *
 * The algorithm above can be re-formulated in more clear way: to
 * be set as new limit, the considered value should not be related
 * to mean more (or less to) than $N$ times. The threshold value is
 * given by $V_{thr} = N$, thus e.g. to restrict value-to-mean fraction
 * for upper limit one need set the threshold to $1/10 = 10^{-1}$.
 *
 * For convinience, rigidness setter/getter is aliased with weakness()
 * method corresponding to $N$ value above.
 */
template<typename FloatT,
         template<typename, typename> class AccumulatorT=NormalAccumulator>
class RigidRange : public Range<FloatT> {
public:
    typedef Range<FloatT> Parent;
    typedef typename Parent::NumType Float;
    typedef AccumulatorT<FloatT, double> Accumulator;
    using typename Parent::LimitIndex;
    using Parent::range_width;
private:
    /// An entity accumulating values.
    Accumulator _accumulator;
    double _rigidness;  ///< A number defining rejection threshold in range (0:1).
    bool _considerAll;  ///< Whether do consider all (even far beyond) values.
protected:
    /// Sets lower limit to provided value only if it fits.
    virtual bool _V_set_lower_limit_to( Float nv ) override {
        if( do_consider_all() ) {
            _accumulator.accumulate( nv );
        }
        if( !Parent::limits_valid() || _accumulator.fits(nv, weakness() ) ) {
            Parent::_V_set_lower_limit_to( _accumulator.boundary_lower( weakness() ) );
            if( !do_consider_all() ) {
                _accumulator.accumulate( nv );
            }
            return true;
        }
        return false;
    }

    /// Sets upper limit to provided value only if it is not too small
    /// comparingly to given rigidness threshold.
    virtual bool _V_set_upper_limit_to( Float nv ) override {
        if( do_consider_all() ) {
            _accumulator.accumulate( nv );
        }
        if( !Parent::limits_valid() || _accumulator.fits( nv, weakness() ) ) {
            Parent::_V_set_upper_limit_to( _accumulator.boundary_upper( weakness() ) );
            if( !do_consider_all() ) {
                _accumulator.accumulate( nv );
            }
            return true;
        }
        return false;
    }
public:
    RigidRange( double rigidness_, bool considerAll=false ) :
            _rigidness( rigidness_ ),
            _considerAll( considerAll ) {}

    /// Rigidness value getter.
    double rigidness() const { return _rigidness; }

    /// Rigidness value setter.
    void rigidness( double nrv ) { _rigidness = nrv; }

    /// $N = 1/V_{thr}$ setter.
    void weakness( double w ) { _rigidness = 1/w; }

    /// $N = 1/V_{thr}$ getter.
    double weakness() const { return 1/_rigidness; }

    /// Do consider all even far beyon values? getter.
    bool do_consider_all() const { return _considerAll; }

    /// Do consider all even far beyon values? setter.
    virtual void do_consider_all( bool v ) { _considerAll = v; }

    /// Accumulator getter (const).
    const Accumulator & accumulator() const { return _accumulator; }

    /// Accumulator getter (mutable).
    Accumulator & accumulator() { return _accumulator; }

    virtual bool extend_to( const FloatT val ) override {
        if( !Parent::extend_to(val) ) {
            if( do_consider_all() ) {
                _accumulator.accumulate( val );
            }
            return false;
        }
        return true;
    }
};


}  // namespace aux
}  // namespace goo

# endif  // GOO_XFORM_TCC

