/*
 *     compress40.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the implementation of the general compression and
 *      decompression wrapper functions.
 *     
 *     Function contracts found in this file.
 */ 

#include "compression.h"
#include "compress40.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "arith40.h"
#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "bitpack.h"

extern void compress40(FILE *input) /* reads PPM, writes compressed image */
{
        /*step 1, read from file*/
        Pnm_ppm sourceImage = Pnm_ppmread(input, uarray2_methods_plain);

        /*step 2, call driver and save returned values*/
        A2Methods_UArray2 compressedUArray2 = compressedImage(sourceImage);

        /*write code words in row major, big endian order using putchar */
        print_compressed(compressedUArray2, uarray2_methods_plain);
        
        uarray2_methods_plain->free(&compressedUArray2);
        Pnm_ppmfree(&sourceImage);
}

extern void decompress40(FILE *input)  /* reads compressed image, writes PPM */
{
        /*step 1, read from file*/
        A2Methods_UArray2 codewords_uarray2 = read_compressed(input);

        /*step 2, call driver and save returned values*/
        A2Methods_UArray2 decomp_uarray2 = decompressedImage(codewords_uarray2);

        /*step 3, write to stdout*/
        print_decompressed(decomp_uarray2);

        uarray2_methods_plain->free(&codewords_uarray2);
}