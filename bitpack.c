/*
 *     bitpack.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the implementation of the bitpack interface which
 *      handles placing signed and unsigned integer values into one 32 bit 
 *      integer.
 *     
 */ 

#include "bitpack.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "except.h"

Except_T Bitpack_Overflow = { "Overflow packing bits" };

uint64_t left_shift(uint64_t n, unsigned shift);
uint64_t right_shiftu(uint64_t n, unsigned shift);
int64_t right_shifts(int64_t n, unsigned shift);


/* Bitpack_fitsu
 * Purpose: checks if a given uint can be bitpacked into a given width
 * Parameters: n, value being bitpacked; and width, num of bits to store it in
 * Returns: boolean, whether or not it fits
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        if (width == 0) {
                return false;
        }

        unsigned upper = left_shift((uint64_t) 1, width) - 1;

        if (n > upper) {
                return false;
        }
        else {
                return true;
        }
}


/* Bitpack_fitss
 * Purpose: checks if a given int can be bitpacked into a given width
 * Parameters: n, value being bitpacked; and width, num of bits to store it in
 * Returns: boolean, whether or not it fits
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
        if (width == 0){
                return false;
        }

        int upper = left_shift((uint64_t) 1, width - 1) - 1;
        int lower = left_shift((uint64_t) 1, width - 1) * -1;

        if (n < lower || n > upper) {
                return false;
        }
        else {
                return true;
        }
}


/* Bitpack_getu
 * Purpose: returns a uint64 that was bitpacked into word, by reading out
 *          width bits starting at lsb
 * Parameters: bitpacked word, width of the target value, lsb of target value
 * Returns: a uint64 containing the target value from the word
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(width + lsb <= 64);

        /* Fields of width zero are defined to contain the value zero */
        if (width == 0) {
                return 0;
        }
        
        /* create a sequence of "width" ones */
        uint64_t mask = left_shift((uint64_t) 1, width) - 1;

        /* put extraction sequence in position */
        mask = left_shift(mask, lsb);

        /* use & operator against word to extract the target bits */
        uint64_t maskedVal = word & mask;

        uint64_t value = right_shiftu(maskedVal, lsb);

        return value;
}


/* Bitpack_gets
 * Purpose: returns an int64 that was bitpacked into word, by reading out
 *          width bits starting at lsb
 * Parameters: bitpacked word, width of the target value, lsb of target value
 * Returns: an int64 containing the target value from the word
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= 64);
        assert(width + lsb <= 64);
        
        /* Fields of width zero are defined to contain the value zero */
        if (width == 0) {
                return 0;
        }

        uint64_t mask = left_shift((uint64_t) 1, width) - 1;
        
        mask = left_shift(mask, lsb);

        /* use & operator against word to extract the target bits */ 
        int64_t maskedVal = word & mask;

        /* shift all the way left, then all the way right, to fill with ones */
        maskedVal = left_shift(maskedVal, 64 - (lsb + width));

        int64_t value = right_shifts(maskedVal, 64 - width);
        
        return value;
}


/* Bitpack_newu
 * Purpose: bitpacks a uint64 into a word, using lsb and width to determine
 *          where it goes and how many bits it takes up
 * Parameters: bitpacked word, intended width/lsb of the value, and the value
 * Returns: a uint64 containing the whole updated word
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, 
                      unsigned lsb, uint64_t value)
{
        assert(width <= 64);
        assert(width + lsb <= 64);
        
        bool can_fit = Bitpack_fitsu(value, width);
        if (! can_fit) {
                RAISE(Bitpack_Overflow);
        }

        /* shift some zeroes into place (with all ones elsewhere) */
        uint64_t mask = left_shift((uint64_t) 1, width) - 1;
        mask = ~(left_shift(mask, lsb));

        /* do word & zeroes to clear the space */ 
        uint64_t clearedWord = word & mask;

        /* fill the cleared slot with the value */
        value = left_shift(value, lsb);
        uint64_t newWord = value | clearedWord;

        return newWord;
}


/* Bitpack_news
 * Purpose: bitpacks an int64 into a word, using lsb and width to determine
 *          where it goes and how many bits it takes up
 * Parameters: bitpacked word, intended width/lsb of the value, and the value
 * Returns: a uint64 containing the whole updated word
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, 
                      unsigned lsb, int64_t value)
{
        assert(width <= 64);
        assert(width + lsb <= 64);
        
        bool can_fit = Bitpack_fitss(value, width);
        if (! can_fit) {
                RAISE(Bitpack_Overflow);
        }

        /* shift some zeroes into place (with all ones elsewhere) */
        uint64_t mask = left_shift((uint64_t) 1, width) - 1;
        mask = ~(left_shift(mask, lsb));
        
        /* do word & zeroes to clear the space */ 
        uint64_t clearedWord = word & mask;

        uint64_t modified_val = left_shift(value, 64 - width);
        modified_val = right_shiftu(modified_val, 64 - width);
        modified_val = left_shift(modified_val, lsb);

        uint64_t newWord = modified_val | clearedWord;

        return newWord;
}


/* left_shift
 * Purpose: performs a left shift operation on n, by the amount shift
 * Parameters: uint64 n to be shifted, and the amount of shift to occur
 * Returns: shifted uint64
 */
uint64_t left_shift(uint64_t n, unsigned shift)
{
        assert(shift <= 64);

        uint64_t result;

        if (shift == 64) {
                result = n << 32;
                result = result << 32;
        }
        else {
                result = n << shift;
        }

        return result;
}

uint64_t right_shiftu(uint64_t n, unsigned shift)
{
        assert(shift <= 64);

        uint64_t result;

        if (shift == 64) {
                result = n >> 32;
                result = result >> 32;
        }
        else {
                result = n >> shift;
        }

        return result;
}

int64_t right_shifts(int64_t n, unsigned shift)
{
        assert(shift <= 64);

        int64_t result;

        if (shift == 64) {
                result = n >> 32;
                result = result >> 32;
        }
        else {
                result = n >> shift;
        }

        return result;
}