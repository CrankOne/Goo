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

# include "utest.hpp"

# if !defined(_Goo_m_NO_GSL)

# include <cstring>
# include <goo_xform.tcc>

# include <gsl/gsl_randist.h>

static const double fltCmpPrecision = 1e-12;

/**@file xform.cpp
 * @brief Scalable numerical ranges tests.
 *
 * This file implements testing unit for numerical ranges operations and
 * XForm classes.
 * */

GOO_UT_BGN( NumRanges, "Scalable numeric ranges" ) {  // ranges

{  
    // Check generic logic:
    //
    goo::aux::Range<double> range;
    _ASSERT( !range.is_initialized( aux::AbstractRange::LimitIndex::min ),
        "Uninitialized range manifests itself as initialized one (min)." );
    _ASSERT( !range.is_initialized( aux::AbstractRange::LimitIndex::max ),
        "Uninitialized range manifests itself as initialized one (max)." );
    _ASSERT( !range.is_initialized( aux::AbstractRange::LimitIndex::both ),
        "Uninitialized range manifests itself as initialized one (both)." );
    _ASSERT( !range.limits_valid(),
        "Uninitialized range manifests itself as a valid one." );
    _ASSERT( !range.is_in( 0. ),
        "Value have not to be covered by uninitialized range." );

    range.upper( 1.5e1 );
    _ASSERT( !range.is_initialized( aux::AbstractRange::LimitIndex::min ),
        "Range with initialized upper limit manifests itself as initialized one (min)." );
    _ASSERT(  range.is_initialized( aux::AbstractRange::LimitIndex::max ),
        "Range with initialized upper limit manifests itself as uninitialized one (max)." );
    _ASSERT( !range.is_initialized( aux::AbstractRange::LimitIndex::both ),
        "Range with initialized upper limit manifests itself as initialized one (both)." );
    _ASSERT( !range.limits_valid(),
        "Range with initialized upper limit manifests itself as a valid one." );

    range.lower( -2.32e1 );
    _ASSERT( range.is_initialized( aux::AbstractRange::LimitIndex::min ),
        "Initialized range manifests itself as uninitialized one (min)." );
    _ASSERT( range.is_initialized( aux::AbstractRange::LimitIndex::max ),
        "Initialized range manifests itself as uninitialized one (max)." );
    _ASSERT( range.is_initialized( aux::AbstractRange::LimitIndex::both ),
        "Initialized range manifests itself as uninitialized one (both)." );
    _ASSERT( range.limits_valid(),
    "Initialized range manifests itself as a invalid one." );

    // Check width/scales/normalization
    //
    _ASSERT(  range.is_in( 0. ),
        "Value have to be covered by range." );
    _ASSERT( !range.is_in( 5e3 ),
        "Value have not to be covered by range." );

    for( UByte i = 0; i < 200; ++i ) {
        const double normedValue_src = rand()/double(RAND_MAX);
        double denormedValue = range.denorm(normedValue_src),
               normedValue = range.norm(denormedValue)
               ;
        _ASSERT( normedValue <= 1. && normedValue >= 0.,
            "Norm procedure yields value out of [0:1] range." )
        _ASSERT( denormedValue <= range.upper() && denormedValue >= range.lower(),
            "Denormed value exceeds range." );
        _ASSERT( fabs(normedValue_src - normedValue) <= fltCmpPrecision,
            "Denormalization/normalization cycle yields wrong value (%e != %e, prec. %e).",
            normedValue_src, normedValue, fltCmpPrecision );
    }

    // Direct extension checks
    //
    for( UInt i = 0; i < 1e3; ++i ) {
        const double value = 200*(.5 - rand()/double(RAND_MAX));
        range.extend_to( value );
    }
    os << "Extension reliability " << range.range_width()/200 << "%" << std::endl;
    _ASSERT( range.upper() >= 60,  // at least to this
        "Extension method didn't cause the range to be really extended: %e <  50.",
        range.upper() );
    _ASSERT( range.lower() <=-60,  // at least to this
        "Extension method didn't cause the range to be really extended: %e > -50.",
        range.upper() );
    _ASSERT( (range.range_width()/200 > 0.8),
        "Extension doesn't exceed reliability threshold of 80%%: width=%e vs expected 200.",
        range.range_width() );
}

{   // Check rigid range algorithm
    //

    aux::RigidRange<double> rRange(1e-1, false);
    {
        const size_t toGenerate = 1000;
        const double gaussianSigma = 15.,
                     gaussianMean = 100
                     ;
        gsl_rng * rng = gsl_rng_alloc( gsl_rng_default );

        os << "Generated gaussian values (" << toGenerate << "):" << std::endl;
        for( UShort i = 0; i < toGenerate; ++i) {
            double val = gaussianMean + gsl_ran_gaussian( rng, gaussianSigma );
            # if 1
            if( rand() < .1*RAND_MAX/10. ) {
                val *= 10;
            }
            # endif
            os << " val " << val;
            if( rRange.extend_to( val ) ) {
                os << " adjusted (now ["
                   << rRange.lower() << ", "
                   << rRange.upper() << "])."
                   ;
            } else {
                os << " ignored ";
                if( rRange.is_in(val) ) {
                    os << "(inside of [" << rRange.lower() << ", " << rRange.upper() << "])";
                } else {
                    os << "(" ESC_BLDRED "far beyond" ESC_CLRCLEAR ")";
                }
            }
            os << std::endl;
        }
        os << std::endl;

        _ASSERT( rRange.is_initialized(),
                 "Range is uninitialized after filling with gaussian set." )

        os << "Range limits now is [" << rRange.lower()
                              << ", " << rRange.upper()
                              << "]"
                              << ", width: " << rRange.range_width()
                              << ", mean: " << rRange.accumulator().cache().mean
                              << std::endl
                              ;
        os << "Accumulator state : " << rRange.accumulator() << std::endl
           << " real distribution parameters : mean = " << gaussianMean << ", "
                                             "sigma = " << gaussianSigma
           << std::endl;


        gsl_rng_free( rng );
    }
}

} GOO_UT_END( NumRanges )

# endif  // _Goo_m_NO_GSL
