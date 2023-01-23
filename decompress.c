/*
 *     decompression.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the implementation of the decompression specific 
 *      functions whos declarations are included in compression.h
 *      All decompression apply functions and input/output functions are 
 *      defined in this file
 *     
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
#include "arith40.h"
#include "bitpack.h"

/* decompressedImage
 * Purpose: decompression driver function: takes in a regular image and carries
 *          out the steps of decompression, returning a decompressed image 
 * Parameters: pixel UArray from source image
 * Returns: Decompressed Image as a UArray2 of Pnm_rgb structs
 */
A2Methods_UArray2 decompressedImage(A2Methods_UArray2 codewords_uarray2)
{

        A2Methods_T methods_plain = uarray2_methods_plain;
        A2Methods_T methods_blocked = uarray2_methods_blocked;
        A2Methods_mapfun *mapRow = methods_plain->map_default;
        A2Methods_mapfun *mapBlocked = methods_blocked->map_default;

        int width = methods_plain->width(codewords_uarray2);
        int height = methods_plain->height(codewords_uarray2);

                        /*DECOMPRESS STEP 5: codeword --> DCTInt*/
        plain_cl source_cl = new_plain_cl(codewords_uarray2, methods_plain, 1);
        A2Methods_UArray2 DCTInt_decompressed = methods_plain->new(width, 
                                        height, sizeof(struct DCT_uint));      
        assert(DCTInt_decompressed != NULL);
        mapRow(DCTInt_decompressed, unpack_codeword, source_cl);
        
                        /*DECOMPRESS STEP 4: DCTInt --> Float*/
        A2Methods_UArray2 DCTFloat_decompressed = methods_plain->new(width, 
                                        height, sizeof(struct DCT_float));
        assert(DCTFloat_decompressed != NULL);
        source_cl->array2 = DCTInt_decompressed;
        mapRow(DCTFloat_decompressed, DCTint_to_float, source_cl);

        /* from now on, our uarray2 is storing single cells as 2x2 blocks, 
         * so we must update the overall width and height variables to account 
         * for that change 
         */
        width = width * 2;
        height = height * 2;

                        /*DECOMPRESS STEP 3: DCTFloat --> compvid*/
        A2Methods_UArray2 compVids_blocked_decompressed = 
                methods_blocked->new_with_blocksize(width, height, 
                                                sizeof(struct compVid), 2);
        assert(compVids_blocked_decompressed != NULL);
        source_cl->array2 = DCTFloat_decompressed;
        mapBlocked(compVids_blocked_decompressed, DCT_to_compVid, source_cl);

                        /*DECOMPRESS STEP 2.5: blocked -> plain */
        A2Methods_UArray2 compVids_plain_decomp = methods_plain->new(width, 
                                            height, sizeof(struct compVid));
        assert(compVids_plain_decomp != NULL);
        source_cl->array2 = compVids_blocked_decompressed;
        source_cl->methods = methods_blocked;
        mapRow(compVids_plain_decomp, blocked_to_plain, source_cl);
        
                        /*DECOMPRESS STEP 2: compvids -> RGB floats*/
        source_cl->array2 = compVids_plain_decomp;
        source_cl->methods = methods_plain;
        A2Methods_UArray2 RGBFloat_decomp = methods_plain->new(width, height, 
                                                sizeof(struct rgbFloat));
        assert(RGBFloat_decomp != NULL);
        mapRow(RGBFloat_decomp, compVid_to_RGBFloat, source_cl);
                
                        /*DECOMPRESS STEP 1: RGB float -> Int*/
        source_cl->array2 = RGBFloat_decomp;
        A2Methods_UArray2 RGBInt_decomp = methods_plain->new(width, height, 
                                                    sizeof(struct Pnm_rgb));
        assert(RGBInt_decomp != NULL);
        mapRow(RGBInt_decomp, RGB_float_to_int, source_cl);

        methods_plain->free(&DCTInt_decompressed);
        methods_plain->free(&DCTFloat_decompressed);
        methods_blocked->free(&compVids_blocked_decompressed);
        methods_plain->free(&compVids_plain_decomp);
        methods_plain->free(&RGBFloat_decomp);

        FREE(source_cl);
        
        return RGBInt_decomp;
}


/* RGB_float_to_int
 * Purpose: Converts a uarray of RGBfloat structs to a UArray of RGBint structs
 * Parameters: 
 * Returns: N/A
 */
void RGB_float_to_int(int col, int row, A2Methods_UArray2 end_array, 
                      void *elem, void *cl) 
{
        (void) end_array;
        
        /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = start->methods;
        A2Methods_UArray2 start_array = start->array2; 
        
        rgbFloat currFloat = methods->at(start_array, col, row);
        int r = (int) (currFloat->red * 255);
        int g = (int) (currFloat->green * 255);
        int b = (int) (currFloat->blue * 255);
        
        r = squash_into_range(0, 255, r);
        g = squash_into_range(0, 255, g);
        b = squash_into_range(0, 255, b);

        struct Pnm_rgb *currInt;
        currInt = (Pnm_rgb) elem;
        
        currInt->red = r;
        currInt->green = g;
        currInt->blue = b;
}
 
/* compVid_to_RGBFloat
 * Purpose: Apply function to be called via map that carries out the step of
 *               decompression that converts compvid structs to RGB floats
 * Parameters: 
 * Returns: N/A
 */
void compVid_to_RGBFloat(int col, int row, A2Methods_UArray2 end_array, 
                         void *elem, void *cl)
{
        (void) end_array;

         /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = start->methods;
        A2Methods_UArray2 start_array = start->array2; 
        
        compVid currCompVid = methods->at(start_array, col, row);
        float y = currCompVid->y;
        float pb = currCompVid->pb;
        float pr = currCompVid->pr;

        float r = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float g = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float b = 1.0 * y + 1.772 * pb + 0.0 * pr;

        struct rgbFloat *currElem;
        currElem = (rgbFloat) elem;

        currElem->red = r;
        currElem->green = g;
        currElem->blue = b;
}

/* blocked_to_plain
 * Purpose: Apply function to be called via map that carries out the step of
 *               decompression that converts between a uarray2b and a uarray2
 * Parameters: closure holds the source blocked array
 * Returns: N/A
 */
void blocked_to_plain (int col, int row, A2Methods_UArray2 end_array, 
                       void *elem, void *cl)
{
        (void) end_array;

        /* unpack the closure (blocked)*/
        struct blocked_cl *start = cl;
        A2Methods_T methods = uarray2_methods_blocked;
        A2Methods_UArray2 start_array = start->array2; 
        assert(start_array != NULL);

        compVid currCompVid = methods->at(start_array, col, row); 
        assert(currCompVid != NULL);
        struct compVid *currElem;
        currElem = (compVid) elem;

        *currElem = *currCompVid;
}

/* DCT_to_compVid
 * Purpose: Apply function to be called via map that carries out the step of
 *               decompression that converts between a uarray2 of DCT structs
 *                to a uarray2 of compVids structs
 * Parameters: 
 * Returns: N/A
 */
void DCT_to_compVid(int col, int row, A2Methods_UArray2 dest_array, 
                    void *elem, void *cl)
{
        (void) dest_array;
        
        /* unpack the closure */
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 start_array = start->array2; 
        
        struct compVid *destCompVid;
        destCompVid = (compVid) elem;
        
        DCT_float currDCT = methods->at(start_array, col / 2, row / 2);
        compVid tempCompVid = createCompVid(currDCT, col, row);

        *destCompVid = *tempCompVid;

        FREE(tempCompVid);
}

/* DCTint_to_Float
 * Purpose: Apply function to be called via map that carries out the step of
 *               decompression that converts between uarray2 of DCTint structs
 *               and a uarray2 of DCTfloat structs
 * Parameters: 
 * Returns: N/A
 */void DCTint_to_float(int col, int row, A2Methods_UArray2 dest_array, 
                        void *elem, void *cl)
{
        (void) dest_array;

        /*unpack closure*/
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 start_array = start->array2; 

        /* get destination DCT int cell*/
        struct DCT_float *destDCTFloat;
        destDCTFloat = (DCT_float) elem;

        /*pre-quantized*/
        DCT_uint currDCTuInt = methods->at(start_array, col, row); 

        destDCTFloat->a = ((float) currDCTuInt->a) / 63.0;

        destDCTFloat->b = unquantizeBCD(currDCTuInt->b);
        destDCTFloat->c = unquantizeBCD(currDCTuInt->c);
        destDCTFloat->d = unquantizeBCD(currDCTuInt->d);

        destDCTFloat->avgPb = Arith40_chroma_of_index(currDCTuInt->pb);
        destDCTFloat->avgPr = Arith40_chroma_of_index(currDCTuInt->pr); 
}

/* unpack_codeword
 * Purpose: Apply function to be called via map that carries out the step of
 *               decompression that converts between a uarray2 of uint32_ts 
 *               that represent codeword and unpackes them into a uarray2 of 
 *               DCTint structs
 * Parameters: 
 * Returns: N/A
 */
void unpack_codeword(int col, int row, A2Methods_UArray2 dest_array, 
                     void *elem, void *cl)
{
        (void) dest_array;

        /*unpack closure*/
        struct plain_cl *start = cl;
        A2Methods_T methods = uarray2_methods_plain;
        A2Methods_UArray2 start_array = start->array2;

        /* get destination DCT int cell*/
        struct DCT_uint *destDCT;
        destDCT = (DCT_uint) elem;

        uint32_t *currCodeword = methods->at(start_array, col, row);
        
        /* unpack everything from the codeword */ 
        destDCT->a = Bitpack_getu(*currCodeword, 6, 26);
        destDCT->b = Bitpack_gets(*currCodeword, 6, 20);
        destDCT->c = Bitpack_gets(*currCodeword, 6, 14);
        destDCT->d = Bitpack_gets(*currCodeword, 6, 8);
        destDCT->pb = Bitpack_getu(*currCodeword, 4, 4);
        destDCT->pr = Bitpack_getu(*currCodeword, 4, 0);
}

/* read_compressed
 * Purpose: read in from a file to make a uarray of uint32-ts
 *          the header, save the width and height, and make a new UArray2
 *          of width x height to store uint 32s and map onto it 
 * Parameters: 
 * Returns: N/A
 */
UArray2_T read_compressed(FILE *fp)
{
        unsigned height, width;

        /* read in header */
        int read = fscanf(fp, "COMP40 Compressed image format 2\n%u %u", 
                                                        &width, &height);
        assert(read == 2);
        int c = getc(fp);
        assert(c == '\n');
        
        A2Methods_UArray2 codewords_uarray2 = uarray2_methods_plain->new(width,
                                                     height, sizeof(uint32_t));
        
        /* read codewords from file and store in codewords_uarray2 */
        uarray2_methods_plain->map_row_major(codewords_uarray2,
                                             read_a_codeword, fp);

        return codewords_uarray2;
}

/* read_a_codeword
 * Purpose: Apply function that reads in 4 chars from a file at a time and 
 *              stores them in a uint32_t to be stored in a uarray2 of
 *              codewords
 * Parameters: 
 * Returns: N/A
 */
void read_a_codeword(int col, int row, A2Methods_UArray2 dest_array, 
                     void *elem, void *cl)
{
        (void) col;
        (void) row;
        (void) dest_array;
        
        /* UNPACK CLOSURE => assign to file ptr */
        FILE *fp = (FILE*) cl;

        /* get destination DCT int cell*/
        uint32_t *codeword;
        codeword = (uint32_t*) elem;

        unsigned char char1 = fgetc(fp);
        assert(feof(fp) == 0);
        unsigned char char2 = fgetc(fp);
        assert(feof(fp) == 0);
        unsigned char char3 = fgetc(fp);
        assert(feof(fp) == 0);
        unsigned char char4 = fgetc(fp);
        assert(feof(fp) == 0);

        uint64_t convertedChar1 = (uint64_t) char1;
        uint64_t convertedChar2 = (uint64_t) char2;
        uint64_t convertedChar3 = (uint64_t) char3;
        uint64_t convertedChar4 = (uint64_t) char4;

        *codeword = Bitpack_newu(*codeword, 8, 24, convertedChar1);
        *codeword = Bitpack_newu(*codeword, 8, 16, convertedChar2);
        *codeword = Bitpack_newu(*codeword, 8, 8, convertedChar3);
        *codeword = Bitpack_newu(*codeword, 8, 0, convertedChar4);
}

/* print_decompressed
 * Purpose: Print the final deompressed image by constructing a Pnm_ppm struct 
 * Parameters: UArray of Pnm_rgbs to make into a Pnm_ppm
 * Returns: N/A
 */
void print_decompressed(A2Methods_UArray2 rgb_decomp_uarray2)
{
        Pnm_ppm image;
        
        NEW(image);
        image->pixels = rgb_decomp_uarray2;
        image->width = uarray2_methods_plain->width(rgb_decomp_uarray2);
        image->height = uarray2_methods_plain->height(rgb_decomp_uarray2);
        image->denominator = 255;
        image->methods = uarray2_methods_plain;

        /* print image to stdout */
        Pnm_ppmwrite(stdout, image);

        Pnm_ppmfree(&image);
}