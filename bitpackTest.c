/*
 *     bitpackTest.c
 *     By Gillian Feder (gfeder01) and Adam Bernstein (aberns07), October 2022
 *     Arith
 *
 *     This file contains the unit test functions for bitpack.c
 *     
 */ 


#include "bitpack.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "except.h"

void fits_tests();
void test_bitpack_fitsu(uint64_t n, unsigned width);
void test_bitpack_fitss(int64_t n, unsigned width);
void get_tests();
void test_bitpack_getu(uint64_t word, unsigned width, unsigned lsb);
void test_bitpack_gets(uint64_t word, unsigned width, unsigned lsb);
void new_tests();
void test_bitpack_newu(uint64_t word, unsigned width, 
                       unsigned lsb, uint64_t value);
void test_bitpack_news(uint64_t word, unsigned width, 
                       unsigned lsb, int64_t value);

int main()
{
        fits_tests();

        get_tests();

        new_tests();

        return 0;
}

/* fits_tests
 * Purpose: runs tests on bitpack_fitsu and bitpack_fitss
 * Parameters: n/a
 * Returns: n/a
 */
void fits_tests()
{
        //unsigned

        printf("\n----FITS UNSIGNED TESTING----\n");
        test_bitpack_fitsu(7, 3); // expect true
        test_bitpack_fitsu(8, 3); // expect false
        test_bitpack_fitsu(17, 4); // expect false
        test_bitpack_fitsu(0, 5); // expect true

        //signed
        printf("\n----FITS SIGNED TESTING----\n");
        test_bitpack_fitss(1, 3); // expect true
        test_bitpack_fitss(4, 3); // expect false
        test_bitpack_fitss(-4, 3); // expect true

}

/* test_bitpack_fitsu
 * Purpose: runs a single test on bitpack_fitsu, prints outcome
 * Parameters: value n, width of bits in which to test if it will fit
 * Returns: n/a
 */
void test_bitpack_fitsu(uint64_t n, unsigned width)
{
        bool tf = Bitpack_fitsu(n, width);
        
        if (tf) {
                printf("YES, %lu fits in %u bits!\n", n, width);
        }
        else {
                printf("NO, %lu does not fit in %u bits (sad)\n", n, width);
        }
}

/* test_bitpack_fitss
 * Purpose: runs a single test on bitpack_fitss, prints outcome
 * Parameters: value n, width of bits in which to test if it will fit
 * Returns: n/a
 */
void test_bitpack_fitss(int64_t n, unsigned width)
{
        bool tf = Bitpack_fitss(n, width);
        
        if (tf) {
                printf("YES, %ld fits in %u bits!\n", n, width);
        }
        else {
                printf("NO, %ld does not fit in %u bits (sad)\n", n, width);
        }
}

/* get_tests
 * Purpose: runs tests on bitpack_getu and bitpack_gets
 * Parameters: n/a
 * Returns: n/a
 */
void get_tests()
{
        printf("\n----GET UNSIGNED TESTING----\n");
        // test_bitpack_getu(96, 4, 4); // expect 6
        
        test_bitpack_getu(224, 4, 4); // expect 14
        test_bitpack_getu(0x3f4, 6, 2); // expect 61


        printf("\n----GET SIGNED TESTING----\n");

        test_bitpack_gets(224, 4, 4); // expect -2

        test_bitpack_gets(0x3f4, 6, 2); // expect -3

        for (int i = 0; i < 20; i++) {
                printf("lsb = %d:  ", i);
                test_bitpack_gets(120, 3, i);
        }
}

/* test_bitpack_getu
 * Purpose: runs a single test on bitpack_getu, prints outcome
 * Parameters: word, width of target value, lsb of target value
 * Returns: n/a
 */
void test_bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        uint64_t value = Bitpack_getu(word, width, lsb);
        printf("value from getter is %lu\n", value);
}

/* test_bitpack_gets
 * Purpose: runs a single test on bitpack_gets, prints outcome
 * Parameters: word, width of target value, lsb of target value
 * Returns: n/a
 */
void test_bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        int64_t value = Bitpack_gets(word, width, lsb);
        printf("value from getter is %ld\n", value);
}

/* new_tests
 * Purpose: runs tests on bitpack_newu and bitpack_news
 * Parameters: n/a
 * Returns: n/a
 */
void new_tests()
{
        printf("\n--NEW UNSIGNED TESTING--\n");

        // should result in uncaught exception
        // test_bitpack_news(120, 3, 2, 100); 

        for (int i = 0; i <= 20; i++) {
                printf("value = %d:  ", i);
                test_bitpack_newu(573, 11, i, i);
        }

        printf("\n--NEW SIGNED TESTING--\n");

        for (int i = -20; i <= 20; i++) {
                printf("value = %d:  ", i);
                test_bitpack_news(121, 6, 3, i);
        }
}

/* test_bitpack_newu
 * Purpose: runs a single test using bitpack_newu, prints the outcome
 * Parameters: word, width/lsb of value, uint64 value itself
 * Returns: n/a
 */
void test_bitpack_newu(uint64_t word, unsigned width, unsigned lsb, uint64_t value)
{
        uint64_t newWord = Bitpack_newu(word, width, lsb, value);
        uint64_t get_value = Bitpack_getu(newWord, width, lsb);
        printf("get u is returning %lu\n", get_value);
}

/* test_bitpack_news
 * Purpose: runs a single test using bitpack_news, prints the outcome
 * Parameters: word, width/lsb of value, int64 value itself
 * Returns: n/a
 */
void test_bitpack_news(uint64_t word, unsigned width, unsigned lsb, int64_t value)
{
        uint64_t newWord = Bitpack_news(word, width, lsb, value);
        int64_t get_value = Bitpack_gets(newWord, width, lsb);
        printf("get s is returning %ld\n", get_value);
}
