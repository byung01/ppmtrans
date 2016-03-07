#include "uarray2b.h"
#include "uarray2.h"
#include <assert.h>
#include "uarray.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
#include "mem.h"

#define T UArray2b_T

static int  find_num_blocks(int dim, int blocksize);
static void iter_block(int r, int c, T array2b, UArray_T block, 
                       void apply(int i, int j, T array2b,
                                        void *elem, void *cl), void *cl);

struct T {
        int width, height;
        int blocksize;
        int size;
        int block_width, block_height;
        UArray2_T blocks;
};

T UArray2b_new(int width, int height, int size, int blocksize)
{
        assert((width > 0) && (height > 0) && (blocksize >= 1));
        
        T array = NEW(array);
        
        int length = blocksize * blocksize;

        /* Initializes the struct */
        array->width        = width;
        array->height       = height;
        array->size         = size;
        array->blocksize    = blocksize;
        array->block_width  = find_num_blocks(width, blocksize);
        array->block_height = find_num_blocks(height, blocksize);

        /* Creates a blocked 2D array */
        array->blocks = UArray2_new(array->block_width, array->block_height,
                                    sizeof(UArray_T));

        /*
         * Loops through each block
         * Each block malloc's a single uarray
         */
        for (int r = 0; r < array->block_height; r++) {
                for (int c = 0; c < array->block_width; c++) {
                        UArray_T *new_array = UArray2_at(array->blocks, c, r);
                        *new_array = UArray_new(length, array->size);
                }
        }
        
        return array;
}

T UArray2b_new_64K_block(int width, int height, int size)
{
        int blocksize = sqrt(64 / size);
        
        /* if a single cell will not fit in 64KB */
        if (blocksize < 1) {
                blocksize = 1;
        }
        
        return UArray2b_new(width, height, size, blocksize);

}

void UArray2b_free(T *array2b)
{
        /* Iterates thorugh the blocks and frees each UArray */
        for (int i = 0; i < (*array2b)->block_height; i++) {
                for (int j = 0; j < (*array2b)->block_width; j++) {
                        UArray_T *temp = UArray2_at((*array2b)->blocks, j, i);
                        UArray_free(temp);
                }
        }

        /* Frees the UArray2 within the struct */
        UArray2_free(&((*array2b)->blocks));

        /* Frees the struct */
        free(*array2b);
}

int UArray2b_width(T array2b)
{
        return array2b -> width;
}

int UArray2b_height(T array2b)
{
        return array2b -> height;
}

int UArray2b_size(T array2b)
{
        return array2b -> size;
}

int UArray2b_blocksize(T array2b)
{
        return array2b -> blocksize;
}

void *UArray2b_at(T array2b, int i, int j)
{
        int blocksize = array2b->blocksize;

        /* Calculates the block that element [j,i] is in */
        UArray_T *temp = UArray2_at(array2b->blocks, i/blocksize, j/blocksize);

        /*
         * Calculates the corresponding index of the single uarray
         * that stores the element [j,i]
         */
        int index = blocksize * (j % blocksize) + (i % blocksize);

        return UArray_at(*temp, index);
}

/* NEED TO WORK ON MAPPING FUNCTION TO ITERATE WITHIN THE BLOCKS*/
void UArray2b_map(T array2b, void apply(int i, int j, T array2b,
                                        void *elem, void *cl), void *cl)
{
        /* Iterates through each block */
        for (int r = 0; r < array2b->block_height; r++) {
                for (int c = 0; c < array2b->block_width; c++) {
                        UArray_T block = *(UArray_T *)UArray2_at
                                                        (array2b->blocks, c, r);
                        iter_block(r, c, array2b, block, apply, cl);
                }
        }
}


/*
 * determines how many blocks will go in the array (adds one if
 * blocksize is not a multiple of width or height
 */
int find_num_blocks(int dim, int blocksize)
{        
        if (dim % blocksize != 0) {
                return ((dim / blocksize) + 1);
        } else {
                return (dim / blocksize);
        }
}


/* Helper function for mapping */
void iter_block(int r, int c, T array2b, UArray_T block,
                void apply(int i, int j, T array2b, void *elem, void *cl),
                void *cl)
{
        /* Calculates the first element of each block [r,c] */
        int row = r * bs;
        int col = c * bs;
        int bs = array2b->blocksize;
        
        for (int i = row; i < row + bs; i++) {
                for (int j = col; j < col + bs; j++) {
                        /* Checks if i and j are within bounds */
                        if ((i < array2b->height) && (j < array2b->width)) {
                                int index = bs * (i % bs) + (j % bs);
                                apply(j, i, array2b,
                                      UArray_at(block, index), cl);
                        }
                }
        }
}
