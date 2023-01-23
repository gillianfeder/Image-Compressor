/*
 *     compression.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the implementation .....
 *     
 *     Function contracts found in this file.
 */ 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pnm.h"
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "compression.h"
#include "mem.h"

int makeEven(int dim);

/* compressedImage
 * Purpose: compression driver function: takes in a regular image and carries
 *          out the steps of compression, returning a compressed image 
 * Parameters: pixel UArray from source image, A2Methods, default map function
 * Returns: Compressed Image as a UArray2_T of UInt_32s
 */
UArray2_T compressedImage(A2Methods_UArray2 originalPixels, 
                           A2Methods_T methods, 
                           A2Methods_mapfun map)
{
        /* if width or height is not even, cut it down */
        int width = makeEven(methods->width(originalPixels));
        int height = makeEven(methods->height(originalPixels));
        
        /* some quicc testing */
        printf("2. width is %d\n", width);
        printf("2. height is %d\n", height);
        //map(originalPixels, printRaster, NULL);

        A2Methods_UArray2 RGB_float_uarray2 = methods->new(width, height, sizeof(struct rgbFloat));
        // assert(RGB_float_uarray2 != NULL);
        
        struct cl_array *startArray;
        NEW(startArray);
        startArray->array2 = originalPixels; 
        // assert(destArray->array2 != NULL);
        startArray->methods = methods;

        // fprintf(stderr, "passes our asserts\n");
        map(RGB_float_uarray2, RGB_int_to_float, startArray);
        // fprintf(stderr, "error is apply \n");

        // printf("(2, 0):  %f %f %f\n", ((rgbFloat)methods->at(RGB_float_uarray2, 2, 0))->red, ((rgbFloat)methods->at(RGB_float_uarray2, 2, 0))->green, ((rgbFloat)methods->at(RGB_float_uarray2, 2, 0))->blue);

        printf("printing entire new raster post float conversion:\n");
        map(RGB_float_uarray2, printRaster, methods);
        
        return NULL;

        // UArray2_T compVid_uarray2 = UArray2_new(image->width, etc.);
        // UArray2_map_row_major(RGB_float_uarray2, float_to_compVid, compVid_uarray2);
        // float_to_compVid takes one pixel, converts it to compVid, stores in other uarray2
}

int makeEven(int dim) 
{
        if (dim % 2 != 0) {
                dim--;
        }
        return dim;
}

void printRaster(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl)
{
        A2Methods_T methods = cl;
        (void) array2;
        (void)cl;
        rgbFloat value;
        value = elem;
        assert(value == ((rgbFloat)methods->at(array2, i, j)));
        printf("cell: [%d, %d]: %f %f %f\n", i, j, value->red, value->green, value->blue);
}
 
//set each cell in the end->array2 to be an rgbfloat struct w/ same "values" as the elem  
void RGB_int_to_float(int col, int row, A2Methods_UArray2 end_array, void *elem, void *cl)
{
        (void) end_array;
        
        /* unpack the closure */
        struct cl_array *start = cl;
        A2Methods_T methods = start->methods;
        A2Methods_UArray2 start_array = start->array2;
        
        fprintf(stderr, "coordinates: [%d, %d]\n", col, row);
        struct rgbFloat *currFloat;
        currFloat = (rgbFloat) elem;
        
        Pnm_rgb currInt = methods->at(start_array, col, row);
        fprintf(stderr, "currInt: %d, %d, %d\n", currInt->red, currInt->green, currInt->blue);
        // fprintf(stderr, "elements blue value: %f\n", ((rgbFloat)elem)->blue);
        // fprintf(stderr, "curr int blue value (original pixels):%f\n", (float)(currInt->blue));
        // ((rgbFloat)elem)->blue = (float)(currInt->blue);
        currFloat->green = (float) (currInt->green);
        currFloat->red = (float) (currInt->red);
        currFloat->blue = (float) (currInt->blue);
        // fprintf(stderr, "blue value in new array:%f", currFloat->blue);
        fprintf(stderr, "currFloat: %f, %f, %f\n", currFloat->red, currFloat->green, currFloat->blue);

        printf("Accessed using 'at' on the dest array: (%d, %d):  %f %f %f\n", col, row, ((rgbFloat)methods->at(end_array, col, row))->red, ((rgbFloat)methods->at(end_array, col, row))->green, ((rgbFloat)methods->at(end_array, col, row))->blue);

        fprintf(stderr, " -- END\n");
}