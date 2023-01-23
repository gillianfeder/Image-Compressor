/*
 *     compression.h
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the interface of all functions required to compress
 *      and decompress and image, including all apply functions, helper
 *      functions, and debugging functions.
 *     
 */ 

#ifndef COMPRESSION_H
#define COMPRESSION_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pnm.h"
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "uarray2.h"
#include "arith40.h"
#include <bitpack.h>

/* --------------- PIXEL STORAGE STRUCTS ------------------ */

typedef struct plain_cl {
        A2Methods_UArray2 array2;
        int denom;
        A2Methods_T methods;
} *plain_cl;

typedef struct block {
        float totalPb, totalPr, y1, y2, y3, y4;
} *block;

typedef struct blocked_cl {
        A2Methods_UArray2 array2;
        A2Methods_T methods;
        struct block *currBlock;
} *blocked_cl;

/* colored pixel (scaled integers) */
typedef struct rgbFloat {
        float red, green, blue;
} *rgbFloat;

typedef struct compVid {
        float y, pb, pr;
} *compVid;


typedef struct DCT_float {
        float avgPb, avgPr, a, b, c, d;
} *DCT_float;

typedef struct DCT_uint {
        unsigned a, pb, pr;
        int b, c, d;
} *DCT_uint;


/* --------------- FUNCTIONS FOR CLIENT TO CALL ------------------ */ 

A2Methods_UArray2 compressedImage(Pnm_ppm sourceImage);
void print_compressed(A2Methods_UArray2 codeword_uarray, A2Methods_T methods);
UArray2_T read_compressed(FILE *fp);
A2Methods_UArray2 decompressedImage(A2Methods_UArray2 codewords_uarray2);
void print_decompressed(A2Methods_UArray2 rgb_decomp_uarray2);

/* ======================================================================
                        COMPRESSION APPLY FUNCTIONS        
   ====================================================================== */

void RGB_int_to_float(int col, int row, A2Methods_UArray2 end_array, void *elem,
                                                        void *cl);
void RGB_to_compVid(int col, int row, A2Methods_UArray2 end_array, void *elem,
                                                                 void *cl);
void plain_to_blocked (int col, int row, A2Methods_UArray2 end_array, 
                                                        void *elem, void *cl);
void compVid_to_DCT(int col, int row, A2Methods_UArray2 end_array, 
                                                        void *elem, void *cl);
void DCTFloat_to_int(int col, int row, A2Methods_UArray2 dest_array, 
                                                        void *elem, void *cl);
void pack_codeword(int col, int row, A2Methods_UArray2 dest_array, 
                                                        void *elem, void *cl);
void output_codeword(int col, int row, A2Methods_UArray2 codeword_uarray, 
                                                        void *elem, void *cl);

/* ======================================================================
                        DECOMPRESSION APPLY FUNCTIONS        
   ====================================================================== */

void RGB_float_to_int(int col, int row, A2Methods_UArray2 end_array, 
                                                        void *elem, void *cl);
void compVid_to_RGBFloat(int col, int row, A2Methods_UArray2 end_array, 
                                                        void *elem, void *cl);
void DCT_to_compVid(int col, int row, A2Methods_UArray2 start_array, 
                                                        void *elem, void *cl);
void blocked_to_plain (int col, int row, A2Methods_UArray2 end_array, 
                                                        void *elem, void *cl);
void DCTint_to_float(int col, int row, A2Methods_UArray2 dest_array, 
                                                        void *elem, void *cl);
void unpack_codeword(int col, int row, A2Methods_UArray2 dest_array, 
                                                        void *elem, void *cl);
void read_a_codeword(int col, int row, A2Methods_UArray2 dest_array, 
                                                        void *elem, void *cl);


/* ======================================================================
                        SHARED HELPER FUNCTIONS        
   ====================================================================== */

/*   UTILITY FUNCTIONS */
int makeEven(int dim);
plain_cl new_plain_cl(A2Methods_UArray2 originalPixels, A2Methods_T methods, 
                                                                int denom);
blocked_cl new_blocked_cl(A2Methods_UArray2 blockedCompVids, 
                                                        A2Methods_T methods);
void reset_block(block newBlock);
int squash_into_range(int lower, int upper, int num);
int quantizeBCD(float x);
float unquantizeBCD(int x);
DCT_float create_DCT(struct block *currBlock);
compVid createCompVid(DCT_float currDCT, int col, int row);

/*    DEBUGGING PRINT FUNCTIONS */
void printFloats(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void printInts(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void printCompVids(int i, int j, A2Methods_UArray2 array2, void *elem, 
                                                                void *cl);
void printDCTs(int i, int j, A2Methods_UArray2 array2, void *elem, void *cl);
void printUnsigned(int i, int j, A2Methods_UArray2 array2, void *elem, 
                                                                void *cl);


#endif