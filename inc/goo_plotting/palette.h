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

# ifndef H_HPH_PALETTE_H
# define H_HPH_PALETTE_H

# include "goo_types.h"

typedef double (*ColorFunction)( double );

# ifndef PLOTTER_PARALLEL_TREATMENT_THR
#   define PLOTTER_PARALLEL_TREATMENT_THR 40
# endif

# ifndef PLOTTER_PARALLEL_TREATMENT_MAX_NTHREADS
#   define PLOTTER_PARALLEL_TREATMENT_MAX_NTHREADS 6
# endif

# ifdef __cplusplus
extern "C" {
# endif

extern ColorFunction plottingPalettes[41];  // 33 13 10
extern UByte nPlottingTicksDefault;

typedef struct {
    UByte R;
    UByte G;
    UByte B;
} goo_RGB;

typedef struct {
    UByte R;
    UByte G;
    UByte B;
    UByte A;
} goo_RGBA;

typedef struct {
    UByte invert;
    ColorFunction RedF;
    ColorFunction GreenF;
    ColorFunction BlueF;
} goo_Palette;

goo_RGB dbl2rgb( double val,
             double min, double max,
             goo_Palette * pt );

# ifdef __cplusplus
}
# endif

# endif  // H_HPH_PALETTE_H


