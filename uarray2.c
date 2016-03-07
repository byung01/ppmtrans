/*  
 *      uarray2.c
 *      By Flora Liu and Bill Yung, 9/20/15
 *      Comp 40 Homework 2: Interfaces, Implementations, and Images
 * 
 *      Purpose: Implementation for 2D UArray
 */

#include "uarray2.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define T UArray2_T

static T     UArray2_init  (T uarray2, int width, int height, int size);
static int   UArray2_length(T  uarray2);
static int   UArray2_get_row(T uarray2, int index);
static int   UArray2_get_col(T uarray2, int index);

struct T {
        int        height;
        int        width;
        int        size;
        UArray_T   array;
};

/* Creates a new 2D UArray */
T UArray2_new(int width, int height, int size) 
{        
        T uarray2 = malloc(sizeof(*uarray2));
        uarray2 = UArray2_init(uarray2, width, height, size);
        
        return uarray2;
}

/* Initializes the 2D UArray */
T UArray2_init(T uarray2, int width, int height, int size) 
{
        uarray2 -> height = height;
        uarray2 -> width  = width;
        uarray2 -> size   = size;
        uarray2 -> array  = UArray_new((width*height), size);
        
        return uarray2;
}

/* Frees the 2D UArray in memory */
void UArray2_free(T *uarray2) 
{
        // Free array
        UArray_free(&((*uarray2) -> array));
        // Free struct containing array   
        free(*uarray2);
}

/* Returns the width of the 2D bit array */
int UArray2_width(T uarray2) {
        return (uarray2 -> width);
}

/* Returns the height of the 2D UArray */
int UArray2_height(T uarray2) {
        return (uarray2 -> height);
}

/* Returns the size of each element in 2D UArray */
int UArray2_size(T uarray2) 
{
        return (uarray2 -> size);
}

/* Returns a pointer to the position provided */
void *UArray2_at(T uarray2, int row, int col) 
{
        int index = row * (uarray2 -> width) + col;
        return UArray_at(uarray2 -> array, index);
}

/* Maps to all elements in row major order */
void UArray2_map_row_major(T uarray2, void apply(int row, int col, T uarray2,
                                                 void *p1, void *p2), void *cl)
{       
        assert(uarray2);
        assert(apply);
        
        for (int i = 0; i < (uarray2 -> width); i++) {
                int k = i;
                for (int j = (uarray2 -> width); k < UArray2_length(uarray2); 
                                                                      k += j) {
                        void *ptr = UArray_at(uarray2 -> array, k);
                        apply(UArray2_get_row(uarray2, k), 
                              UArray2_get_col(uarray2, k),
                              uarray2, ptr, cl);
                }
       }
}

/* Maps to all elements in col major order */
void  UArray2_map_col_major(T uarray2, void apply(int row, int col, T uarray2,
                                                  void *p1, void *p2), void *cl)
{      
       assert(uarray2);
       assert(apply);
              
       for (int i = 0; i < UArray2_length(uarray2); i++) {
                void *ptr = UArray_at(uarray2 -> array, i);
                apply(UArray2_get_row(uarray2, i), 
                      UArray2_get_col(uarray2, i),
                      uarray2, ptr, cl);
        }
}

/* 
 * Returns the length of 1D UArray (hidden from client) 
 * Used in mapping function
 */
int UArray2_length(T uarray2) 
{
        return UArray_length(uarray2->array);
}

/* 
 * Returns the row of an element in 1D UArray (hidden from client) 
 * Used in mapping function
 */
int   UArray2_get_row(T uarray2, int index)
{
        return index / (uarray2 -> width);
}

/* 
 * Returns the col of an element in 1D UArray (hidden from client) 
 * Used in mapping function
 */
int   UArray2_get_col(T uarray2, int index)
{
        return index % (uarray2 -> width);
}