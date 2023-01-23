/*
 *  a2plain.c
 *  By Gillian Feder (gfeder01) and Max Shellist (mshell01), October 2022
 *  locality
 *
 *  Implementation of the methods suite specified for blocked Uarray2s.
 *           
 */

#include <a2plain.h>
#include <stdlib.h>
#include <string.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/

/* Returns a new plain UArray2 with the specified width, height, and size
 */
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/* Returns a new plain UArray2 with the specified width, height, and size
 */
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size, 
                                                        int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/* Frees the plain UArray2 by calling the uarray2.c free function
 */
static void a2free(A2Methods_UArray2 * array2p)
{
	UArray2_free((UArray2_T *) array2p);
}

/* calls the UArray2.c width function to return the width of a plain UArray2
 */
static int width(A2Methods_UArray2 array2)
{
	return UArray2_width(array2);
}

/* calls the UArray2.c height function to return the height of a plain UArray2
 */
static int height(A2Methods_UArray2 array2)
{
	return UArray2_height(array2);
}

/* calls the UArray2.c size function to return the size of a plain UArray2
 */
static int size(A2Methods_UArray2 array2)
{
	return UArray2_size(array2);
}

/* Retruns blocksize of a plain array, which is just 1 
 */
static int blocksize(A2Methods_UArray2 array2)   
{
        (void)array2;
	return 1;
}

/* Returns the cell at the specified index i, j in the UArray2.
 */
static A2Methods_Object *at(A2Methods_UArray2 array2, int i, int j)
{
	return UArray2_at(array2, i, j);
}

static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
         A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,		
        map_col_major,		
        NULL,                   /* map_block_major */
        map_row_major,	        /* map_default */
        small_map_row_major,	
        small_map_col_major,	
        NULL,                   /* small_map_block_major */
        small_map_row_major,	/* small_map_default */
};

/* finally the payoff: here is the exported pointer to the struct */

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
