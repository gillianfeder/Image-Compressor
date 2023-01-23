/*
 *     compression.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *      This file contains the implementation of the compression specific 
 *      functions whos declarations are included in compression.h
 *      All compression apply functions and input/output functions are 
 *      defined in this file
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
#include <math.h>
#include "arith40.h"
#include <stdint.h>
#include <bitpack.h>



/* compressedImage
 * Purpose: compression driver function: takes in a regular image and carries
 *          out the steps of compression, returning a compressed image 
 * Parameters: pixel UArray from source image, A2Methods, default map function
 * Returns: Compressed Image as a UArray2_T of UInt_32s
 */
A2Methods_UArray2 compressedImage(Pnm_ppm sourceImage)
{
        A2Methods_T methods_plain = uarray2_methods_plain;
        A2Methods_T methods_blocked = uarray2_methods_blocked;
        A2Methods_mapfun *mapRow = methods_plain->map_default;
        A2Methods_mapfun *mapBlocked = methods_blocked->map_default;
        A2Methods_UArray2 originalPixels = sourceImage->pixels;
        int denom = (int)sourceImage->denominator;
        
        /* if width or height is not even, cut it down */
        int width = makeEven(methods_plain->width(originalPixels));
        int height = makeEven(methods_plain->height(originalPixels));

        /* create struct to use as closure, store originalPixels inside */
        plain_cl source_cl = new_plain_cl(originalPixels, methods_plain, 
                                          denom);

                        /* COMPRESS STEP 1: RGBInt -> float */
        A2Methods_UArray2 RGB_float_uarray2 = methods_plain->new(width, 
                                                                 height, 
                                                  sizeof(struct rgbFloat));
        assert(RGB_float_uarray2 != NULL);
        mapRow(RGB_float_uarray2, RGB_int_to_float, source_cl);
        
                        /*COMPRESS STEP 2: RGBFloat -> compVid */
        source_cl->array2 = RGB_float_uarray2;
        A2Methods_UArray2 compVid_uarray2 = methods_plain->new(width, 
                                                               height, 
                                                 sizeof(struct compVid));
        mapRow(compVid_uarray2, RGB_to_compVid, source_cl);  
        assert(compVid_uarray2 != NULL);

                        /* COMPRESSION STEP 2.5: plain -> blocked */
        A2Methods_UArray2 compVid_uarray2_blocked = 
                methods_blocked->new_with_blocksize(width, height, 
                                        sizeof(struct compVid), 2);
        source_cl->array2 = compVid_uarray2_blocked;
        assert(compVid_uarray2_blocked != NULL);
        /* row major mapping, source_cl contains destination blocked array */
        mapRow(compVid_uarray2, plain_to_blocked, source_cl);
        /* from now on, our uarray2 is storing 2x2 blocks in a single cell, 
         * so we must update the overall width and height variables to account 
         * for that change 
         */
        width = width / 2;
        height = height / 2;
        
                      /* COMPRESSION STEP 3: CompVid -> DCT */
        A2Methods_UArray2 DCTFloat_uarray2 = methods_plain->new(width, height, 
                                                sizeof(struct DCT_float));
        assert(DCTFloat_uarray2 != NULL);
        blocked_cl endArray = new_blocked_cl(DCTFloat_uarray2, methods_plain);
        mapBlocked(compVid_uarray2_blocked, compVid_to_DCT, endArray);   

                     /* COMPRESSION STEP 4: DCTFloat -> DCT_uInt*/
        A2Methods_UArray2 DCTInt_uarray2 = methods_plain->new(width, height, 
                                                sizeof(struct DCT_uint)); 
        assert(DCTInt_uarray2 != NULL);   
        mapRow(DCTInt_uarray2, DCTFloat_to_int, endArray); /* endarray = start*/

                 /* COMPRESSION STEP 5 DCT_uInt -> codeword*/
        A2Methods_UArray2 codewords_uarray2 = methods_plain->new(width, height, 
                                                        sizeof(uint32_t)); 
        assert(codewords_uarray2 != NULL);
        source_cl->array2 = DCTInt_uarray2;
        source_cl->methods = methods_plain;
        mapRow(codewords_uarray2, pack_codeword, source_cl);

        methods_plain->free(&RGB_float_uarray2);
        methods_plain->free(&compVid_uarray2);
        methods_blocked->free(&compVid_uarray2_blocked);
        methods_plain->free(&DCTFloat_uarray2);
        methods_plain->free(&DCTInt_uarray2);

        FREE(source_cl);
        FREE(endArray->currBlock);
        FREE(endArray);

        return codewords_uarray2;
}


/* RGB_int_to_float
 * Purpose: compression apply function that converts a UArray of RGBfloata  
 * Parameters: 
 * Returns: N/A
 */  
void RGB_int_to_float(int col, int row, A2Methods_UArray2 end_array, void *elem, void *cl)
{
        (void) end_array;
        
        /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = start->methods;
        A2Methods_UArray2 start_array = start->array2;
        int denominator = start->denom;

        Pnm_rgb currInt = methods->at(start_array, col, row);
        float r = ((float) (currInt->red)) / denominator;
        float g = ((float) (currInt->green)) / denominator;
        float b = ((float) (currInt->blue)) / denominator;

        struct rgbFloat *currFloat;
        currFloat = (rgbFloat) elem;
        
        currFloat->red = r;
        currFloat->green = g;
        currFloat->blue = b;
}

/* RGB_to_compVid
 * Purpose: compression apply function that completes the second step of 
 *              compression which converts a UArray of RGBfloats to a uarray 
 *              of compvid structs via mapping between two arrays  
 * Parameters: 
 * Returns: N/A
 */ 
void RGB_to_compVid(int col, int row, A2Methods_UArray2 end_array, void *elem, void *cl)
{
        (void) end_array;

         /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = start->methods;
        A2Methods_UArray2 start_array = start->array2; 
        
        rgbFloat currFloat = methods->at(start_array, col, row);
        float r = currFloat->red;
        float g = currFloat->green;
        float b = currFloat->blue;

        float y = 0.299 * r + 0.587 * g + 0.114 * b;
        float pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
        float pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

        struct compVid *currElem;
        currElem = (compVid) elem;

        currElem->y = y;
        currElem->pb = pb;
        currElem->pr = pr;
}

/* plain_to_blocked
 * Purpose: compression apply function that completes the next step of 
 *              compression which converts a plain UArray to a blocked uarray 
 * Parameters: 
 * Returns: N/A
 */ 
void plain_to_blocked (int col, int row, A2Methods_UArray2 end_array, void *elem, void *cl)
{
        (void) end_array;

        /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_blocked;
        A2Methods_UArray2 start_array = start->array2; 
        assert(start_array != NULL);

        compVid currElem = methods->at(start_array, col, row); 
        assert(currElem != NULL);
        struct compVid *currCompVid;
        currCompVid = (compVid) elem;

        *currElem = *currCompVid;
}

/* compVid_to_DCT
 * Purpose: compression apply function that completes the step of 
 *              compression which converts a UArray of compVid structs  
 *              to a uarray of DCT structs via mapping between two arrays  
 * Parameters: closure hold end (destination) array
 * Returns: N/A
 */ 
void compVid_to_DCT(int col, int row, A2Methods_UArray2 start_array, void *elem, void *cl)
{
        (void) start_array;
        
        /* unpack the closure */
        struct blocked_cl *end = cl;
        A2Methods_T methods = end->methods;
        A2Methods_UArray2 end_array = end->array2; 
        struct block *currBlock = end->currBlock;
        
        struct compVid *currElem;
        currElem = (compVid) elem;
        
        /*update block values with values in current elem */
        currBlock->totalPb += currElem->pb;
        currBlock->totalPr += currElem->pr;
        
        if (row % 2 == 0 && col % 2 == 0){
                currBlock->y1 = currElem->y;
        }
        else if (row % 2 == 0 && col % 2 != 0) {
                currBlock->y2 = currElem->y;
        }
        else if (row % 2 != 0 && col % 2 == 0) {
                currBlock->y3 = currElem->y;
        }
        else {             /* column and row are both odd -> end of block */
                currBlock->y4 = currElem->y;
                DCT_float DCT_from_block = create_DCT(currBlock);
                DCT_float currDCT = methods->at(end_array, col / 2, row / 2);
                *currDCT = *DCT_from_block;
                FREE(DCT_from_block);
                reset_block(currBlock);
        }                             
}

/* compVid_to_DCT
 * Purpose: compression apply function that completes the step of 
 *              compression which converts a UArray of DCTfloat structs  
 *              to a uarray of DCT int struct via mapping between two arrays 
 *              via quantization  
 * Parameters: source array in plain_cl struct passed as closure
 * Returns: N/A
 */
void DCTFloat_to_int(int col, int row, A2Methods_UArray2 dest_array, void *elem,
                                                                 void *cl)
{
        (void) dest_array;
        
        /*unpack closure*/
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 start_array = start->array2; 

        /* get destination DCT int cell*/
        struct DCT_uint *destDCT;
        destDCT = (DCT_uint) elem;
        /*pre-quantized*/
        DCT_float currDCT = methods->at(start_array, col, row); 

        /*CODING A: code a in nine unsigned bits if you multiply by 511 and 
                                                                round.*/
        destDCT->a = (unsigned) round(currDCT->a * 63.0);

        /* CODING B, C, D: multiply by 40 and round using rounding function */
        destDCT->b = quantizeBCD(currDCT->b);
        destDCT->c = quantizeBCD(currDCT->c);
        destDCT->d = quantizeBCD(currDCT->d);

        /*CODING PB AND PR using provided function */
        destDCT->pb = Arith40_index_of_chroma(currDCT->avgPb);
        destDCT->pr = Arith40_index_of_chroma(currDCT->avgPr); 
        
}       

/* pack_codeword
 * Purpose: compression apply function that completes the step of 
 *              compression which converts a UArray of DCTint structs  
 *              to a uarray of uint32_ts  via mapping between two arrays 
 *              packing the values within the struct into a codeword  
 * Parameters: source array in plain_cl struct passed as closure
 * Returns: N/A
 */
void pack_codeword(int col, int row, A2Methods_UArray2 dest_array, void *elem, void *cl)
{
        (void) dest_array;
        /*unpack closure*/
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 start_array = start->array2;

        /* get destination DCT int cell*/
        uint32_t *destWord;
        destWord = (uint32_t*) elem;

        DCT_uint currDCTuInt = methods->at(start_array, col, row); 

        /* bitpack everything */
        *destWord = Bitpack_newu(*destWord, 6, 26, currDCTuInt->a);
        *destWord = Bitpack_news(*destWord, 6, 20, currDCTuInt->b);
        *destWord = Bitpack_news(*destWord, 6, 14, currDCTuInt->c);
        *destWord = Bitpack_news(*destWord, 6, 8, currDCTuInt->d);
        *destWord = Bitpack_newu(*destWord, 4, 4, currDCTuInt->pb);
        *destWord = Bitpack_newu(*destWord, 4, 0, currDCTuInt->pr);
}

/* print_compressed
 * Purpose: Print out the contents of a uarray2 of codewords in row major
 *              order as chars (1 per every 8 bits) to standard output  
 * Parameters: UArray2 to print and methods to use on the UArray
 * Returns: N/A
 */
void print_compressed(A2Methods_UArray2 codeword_uarray, A2Methods_T methods)
{
        int width = methods->width(codeword_uarray);
        int height = methods->height(codeword_uarray);
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        methods->map_row_major(codeword_uarray, output_codeword, NULL);
}

/* output_codeword
 * Purpose: compression apply function that completes the final step of 
 *              compression which outputs the uint32_t codewords to standard
 *              output. 
 * Parameters: source array in plain_cl struct passed as closure
 * Returns: N/A
 */
void output_codeword(int col, int row, A2Methods_UArray2 codeword_uarray, void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) codeword_uarray;
        (void) cl;

        uint32_t *codeword;
        codeword = (uint32_t*) elem;

        uint8_t num1 = Bitpack_getu(*codeword, 8, 24);
        uint8_t num2 = Bitpack_getu(*codeword, 8, 16);
        uint8_t num3 = Bitpack_getu(*codeword, 8, 8);
        uint8_t num4 = Bitpack_getu(*codeword, 8, 0);

        putchar(num1);
        putchar(num2);
        putchar(num3);
        putchar(num4);
}
