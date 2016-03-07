/*
 *                      useuarray2b.c
 *
 *         This program illustrates the use of the uarray2b interface.
 *
 *         Although it will catch some errors in some uarray2b implementations
 *         it is NOT a thorough test program.
 *
 *         NOTE: this program is commented sparsely, as figuring out
 *         what this program does and why the tests it makes matter is
 *         part of the homework assignment.
 *
 *         Author: Noah Mendelsohn
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assert.h"

#include <uarray2b.h>

typedef int number;

const int DIM1 = 10;
const int DIM2 = 10;
const int ELEMENT_SIZE = 4;
const int MARKER = 99;
const int BLOCKSIZE = 4;

void
check_and_print(int i, int j, UArray2b_T a, void *p1, void *p2) 
{
        number *entry_p = p1;

        *((bool *)p2) &= UArray2b_at(a, i, j) == entry_p;

        if ( (i == (DIM1 -1) ) && (j == (DIM2 - 1) ) ) {
                /* we got the corner */
                *((bool *)p2) &= (*entry_p == MARKER);
        }

        printf("ar[%d,%d]\n", i, j);
}

int
main(int argc, char *argv[])
{
        (void)argc;
        (void)argv;

        UArray2b_T test_array;
        bool OK = true;

        test_array = UArray2b_new(DIM1, DIM2, ELEMENT_SIZE, BLOCKSIZE);

        OK &= (UArray2b_width(test_array) == DIM1);
        OK &= (UArray2b_height(test_array) == DIM2);
        OK &= (UArray2b_size(test_array) == ELEMENT_SIZE);
        OK &= (UArray2b_blocksize(test_array) == BLOCKSIZE);
        assert(OK);

        /* Note: we are only setting a value on the corner of the array */
        *((number *)UArray2b_at(test_array, DIM1-1, DIM2-1)) = MARKER;
        
        printf("Trying block major\n");
        UArray2b_map(test_array, check_and_print, &OK);

        UArray2b_free(&test_array);

        printf("The array is %sOK!\n", (OK ? "" : "NOT "));

}
