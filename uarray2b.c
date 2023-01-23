/*
 *  uarray2b.c
 *  By Gillian Feder (gfeder01) and Max Shellist (mshell01), October 2022
 *  locality
 *
 *  Implementation of a blocked UArray2. Individual blocks are represented 
 *  as UArrays.
 *      
 */

#include <assert.h>
#include <math.h>
#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <uarray.h>
#include <uarray2.h>
#include <uarray2b.h>
#include "pnm.h"

struct UArray2b_T {
        int width;
        int height;
        int size;
        int blocksize;

        UArray2_T blocks;
};

#define T UArray2b_T

/* Initialize a new blocked 2D array. Initialize each block as a UArray.
*/
extern T UArray2b_new (int width, int height, int size, int blocksize) 
{
        T arr2b;
        NEW(arr2b);
        assert(arr2b != NULL && blocksize >= 1); 

        arr2b->width     = width; 
        arr2b->height    = height;
        arr2b->size      = size;
        arr2b->blocksize = blocksize;

        /* set blocksize to 1 if each cell is larger than 64KB */
        if (size > 64000) {
                arr2b->blocksize = 1;
        }

        /* initialize blocks array */  
        arr2b->blocks = UArray2_new((int)ceil((double)width  / blocksize), 
                                    (int)ceil((double)height / blocksize), 
                                    sizeof(UArray_T));
        
        /* set each block to a UArray */
        for (int col = 0; col < UArray2_width(arr2b->blocks); col++) {
                for (int row = 0; row < UArray2_height(arr2b->blocks); row++) {

                        UArray_T block = UArray_new(arr2b->blocksize * 
                                        arr2b->blocksize, size);
                        assert(block != NULL);

                        *(UArray_T *)UArray2_at(arr2b->blocks, col, row) = 
                                                                        block;
                }
        }
        return arr2b;
}


/* Initialize a new blocked 2D array with blocksize as large as possible 
 * provided the block occupies at most 64KB (if possible)
 */
extern T UArray2b_new_64K_block(int width, int height, int size) 
{
        return UArray2b_new (width,  height,  size, (int)floor(sqrt(65536.0/
                                                                    size))); 

}

/* Frees all internal structures an the entire UArray2b, by looping through
 * its blocks, freeing them, freeing the entire array, and the struct itself.
 */
extern void UArray2b_free (T *array2b) 
{
        assert(*array2b != NULL);
        for (int row = 0; row < UArray2_height((*array2b)->blocks); row++) {
                for (int col = 0; col < UArray2_width((*array2b)->blocks); 
                                                                col++) {
                        UArray_T *block = UArray2_at(((*array2b)->blocks), col, 
                                                                        row);
                        UArray_free(block);
                }
        }
        UArray2_free(&((*array2b)->blocks));
        FREE(*array2b);
}

/* Return the width of the Uarray2b
 */
extern int UArray2b_width (T array2b) 
{
        assert(array2b != NULL);
        return array2b->width;
}

/* Return the height of the Uarray2b
 */
extern int UArray2b_height (T array2b) 
{
        assert(array2b != NULL);
        return array2b->height;
}

/* Return the size of the elements stored in Uarray2b
 */
extern int UArray2b_size (T array2b) 
{
        assert(array2b != NULL);
        return array2b -> size;
}

/* Return the blocksize of the Uarray2b
 */
extern int UArray2b_blocksize(T array2b)
{
        assert(array2b != NULL);
        return array2b->blocksize;
}


/* Returns a pointer to the cell in the given column and row by first
 * retreiving the block and then calculating the correct index within
 * that block.
 */
extern void *UArray2b_at(T array2b, int column, int row) 
{
        assert(column < array2b->width && row < array2b->height);
        assert(array2b != NULL);

        /* get correct block */
        UArray_T *block = UArray2_at(array2b->blocks, 
                                    column / array2b->blocksize, 
                                    row / array2b->blocksize);

        /* calculate correct index */
        int index = array2b->blocksize * (row % array2b->blocksize) + 
                                         (column % array2b->blocksize);

        return UArray_at(*block, index);
} 

/* Block major mapping function, traverses through the blocks in row major
 * order, and in each block, traverses through cells also in row major
 * order. Takes in the array to be traversed, the apply function, and
 * the closure pointer. 
 */
extern void UArray2b_map(T array2b,
void apply(int col, int row, T array2b,
void *elem, void *cl),
void *cl)
{
        assert(array2b != NULL);

        /* iterate through our 2d array of blocks */
        for (int i = 0; i < UArray2_height(array2b->blocks); i++) {
                for (int j = 0; j < UArray2_width(array2b->blocks); j++) {

                        /* get the current block */
                        UArray_T *block = UArray2_at(array2b->blocks, j, i);

                        /* iterate through the current block */
                        for (int k = 0; k < UArray_length(*block); k++) {
                                
                                /* check if we are in a non-full block in the
                                 * right column, and if current cell is valid
                                 */
                                if ((j == UArray2_width(array2b->blocks) - 1) &&
                                    (array2b->width % array2b->blocksize != 0) 
                                    && (k % array2b->blocksize >= 
                                        array2b->width % array2b->blocksize)) {
                                        /* if not valid, do not apply */
                                        continue;
                                }
                                /* check if we are in a non-full block in the
                                 * bottom row, and if the current cell is valid
                                 */
                                else if (i == UArray2_height(array2b->blocks) 
                                                        - 1 && (array2b->height 
                                        % array2b->blocksize != 0) && 
                                        k == array2b->blocksize *  
                                        (array2b->height % array2b->blocksize)){
                                        /* if not valid, we know we are done
                                         * with the current block
                                         */
                                        break;
                                } 
                                else {
                                        /* get the cell coordinates from the
                                         * block coordinates and position within
                                         * the block 
                                         */
                                        int col = array2b->blocksize * j + k % 
                                                        array2b->blocksize;
                                        int row = array2b->blocksize * i + k / 
                                                        array2b->blocksize;

                                        apply(col, row, array2b, 
                                                UArray_at(*block, k), cl);
                                }
                        }
                }
        } 
}