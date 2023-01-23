/*
 *     testing.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains a testing framework that we used throughout the
 *      assignment to check each step.
 *     
 */ 

#include "compression.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pnm.h>
#include <mem.h>


int main() {
        struct Pnm_rgb testRGB = {0, 0, 0};  
        (void) testRGB;
        FILE* filep;

        filep = fopen("flowersCol0.ppm", "r");
        Pnm_ppm sourceImage = Pnm_ppmread(filep, uarray2_methods_plain);

        //step 1 of compression
        UArray2_T compressed = compressedImage(sourceImage);
        
        //step 1 of decompression
        UArray2_T decompressed = decompressedImage(compressed);
        
        
        // FILE *outputFile = fopen(argv[1], "w");
        // print_image(compressed, outputFile);

        print_decompressed(decompressed);
        
        return 0;
}

