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


# ifndef H_GOO_NICENUMBERS_H
# define H_GOO_NICENUMBERS_H

/**@file goo_nicenum.h
 * @brief Nice number algorithm implementation originally described
 * by Paul S. Heckbert in "Graphics Gems I" edited by S. Glassner (II.2).
 *
 * Labels are stored as single-linked list.
 * */

# include <stdint.h>

/**@struct LabelsList
 * @brief A node structure containing a label with formatted text and
 * exact numeric value. */
struct LabelsList {
    double position;
    char labelStr[16];
    struct LabelsList * nextPtr;
};

/** Allocates new uninitialized LabelsList instance. */
struct LabelsList * alloc_labels_list_node();

/** Frees LabelsList node. */
void free_labels_list_node( struct LabelsList * );

/**Frees list of labels previously allocated by loose_label() */
void free_labels_list( struct LabelsList * );

/**Inserts labels list node B after labels list node A.*/
void insert_labels_list_node( struct LabelsList * A, struct LabelsList * B );

/**Find a "nice" number approximately equal to x.
 * Round the number if round == true,
 * take ceiling if round == false.
 */
double nicenum( double x, uint8_t doRound );

/**Label the data range from min to max loosely
 * (tight method is similar).
 */
uint16_t loose_label( double * rangeMinPtr, double * rangeMaxPtr,
                      uint16_t * nTicksPtr,
                      struct LabelsList ** lstHead );

# endif  /* H_GOO_NICENUMBERS_H */

