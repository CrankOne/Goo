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

/**@file palette.h
 * @brief C-header declaring palette routines
 *
 * File containing declaration of types and routines performing mapping from real
 * value to RGB(A)-color. The "standard" color functions for palette declared here
 * as an array of callbacks are similar to ones that can be found in gnuplot.
 */

# include "goo_types.h"

# ifdef __cplusplus
extern "C" {
# endif

/** A color function type performing match from real value
 * to color ([0..1] -> [0..1]).*/
typedef double (*ColorFunction)( double );

/** Pre-defined ("standard") color functions array corresponding to "gnuplot". */
extern ColorFunction plottingPalettes[41];  /* 33 13 10 */

/** A basic Goo RGB color type. */
typedef union {
    struct { UByte r,g,b; } byChannel;
    UByte byte[3];
} goo_RGB;

/** A basic Goo HSV color type. */
typedef union {
    struct { Float8 h,s,v; } byComponent;
    Float8 byte[3];
} goo_HSV;

/** A basic goo RGBA color type (RGB with alpha-channel). */
typedef union {
    struct { UByte r,g,b,a; } byChannel;
    UByte byte[4];
} goo_RGBA;

/** A basic RGBA palette structure containing pointers to color functions. */
typedef struct {
    ColorFunction RedF;
    ColorFunction GreenF;
    ColorFunction BlueF;
} goo_Palette;

/**@brief Value-to-RGB color conversion function.
 *
 * Takes value with bounds, palette, and produces the RGB struct.
 * */
goo_RGB real_to_rgb( double val,
                     double min,
                     double max,
                     goo_Palette * pt );

# ifdef __cplusplus
}
# endif

# endif  // H_HPH_PALETTE_H


