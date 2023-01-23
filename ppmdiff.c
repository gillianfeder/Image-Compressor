/*
 *     ppmdiff.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the implementation of ppmdiff, which calculates
 *      the difference between two ppm files used for testing the
 *      accuracy of compression and decompression.
 *     
 */ 


#include <stdio.h>
#include <stdlib.h>
#include "pnm.h"
#include "a2methods.h"
#include "assert.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "stdbool.h"
#include "math.h"

Pnm_ppm read_image(char *file_name, A2Methods_T methods); 
bool valid_dimensions(Pnm_ppm image1, Pnm_ppm image2);
float image_difference (Pnm_ppm image1, Pnm_ppm image2);
int min(int a, int b);
int square(int x);

int main(int argc, char* argv[]) {
        if (argc != 3) {
                fprintf(stderr, "Incorrect number of command line arguments\n");
                exit(EXIT_FAILURE);
        }
        
        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        Pnm_ppm image1 = read_image(argv[1], methods);
        Pnm_ppm image2 = read_image(argv[2], methods);
        if (!valid_dimensions(image1, image2)) {
                exit(EXIT_FAILURE);
        }
        float diff_val = image_difference(image1, image2);

        printf("DIFF VAL: %f\n", diff_val);

        Pnm_ppmfree(&image1);
        Pnm_ppmfree(&image2);

        return(0);
}

/* 
* Description: read a ppm image frm a file and return it as a pnmppm
* Input: file name and methods 
* Returns: Pnm_ppm
*/
Pnm_ppm read_image(char *file_name, A2Methods_T methods) 
{
        FILE *filep = fopen(file_name,"r");
        assert(filep != NULL);

        return Pnm_ppmread(filep, methods);
} 

/* 
* Description: Check that there is a maximum difference of 1 between the 
*               dimensions of the two pnm_ppms
* Input: The two Pnm_ppms being compared 
* Returns: boolean representing success of dimension read
*/
bool valid_dimensions(Pnm_ppm image1, Pnm_ppm image2)
{
        bool valid = true;
        if (image1->width - image2->width > 1) {
                fprintf(stderr, "Width difference greater than 1\n");
                printf( "1.0\n");
                valid = false;
        }

        if (image1->height - image2->height > 1) {
                fprintf(stderr, "Height difference greater than 1\n");
                printf("1.0\n");
                valid = false;
        }
        return valid;
}

/* 
* Description: calculate the difference between the two images
* Input: The two Pnm_ppms being compared 
* Returns: Value representing image difference
*/
float image_difference (Pnm_ppm image1, Pnm_ppm image2) 
{
        int width = min(image1->width, image2->width);
        int height = min(image1->height, image2->height);
        float redDiff, blueDiff, greenDiff, sum = 0;
        
        for (int j = 0; j < height; j++) {
                for (int i = 0; i < width; i++) {
                        Pnm_rgb image1pixel = image1->methods->at(image1->pixels, i, j);
                        Pnm_rgb image2pixel = image2->methods->at(image2->pixels, i, j);
                        redDiff = square(image1pixel->red - image2pixel->red);
                        greenDiff = square(image1pixel->green - image2pixel->green);
                        blueDiff = square(image1pixel->blue - image2pixel->blue);
                        sum += (redDiff + greenDiff + blueDiff);

                }
        }

        float diff_val = sqrt(sum / (3 * width * height));

        return diff_val;
}

/* 
* Description: find the minimum of two numbers
* Input: two integers to compare
* Returns: the minimum of the two numbers
*/
int min(int a, int b) 
{
        if (a > b) {
                return b;
        }
        return a;
}

/* 
* Description: find the square of a number x
* Input: number to square
* Returns: squared number
*/
int square(int x)
{
        return x * x;
}