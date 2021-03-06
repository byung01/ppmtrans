/*  
 *      uarray2.h
 *      By Flora Liu and Bill Yung, 9/20/15
 *      Comp 40 Homework 2: Interfaces, Implementations, and Images
 * 
 *      Purpose: Interface for 2D UArray57
 */

#include <uarray.h>
#ifndef UARRAY2_H_INCLUDED
#define UARRAY2_H_INCLUDED

#define T UArray2_T
typedef struct T *T;

extern T     UArray2_new   (int width, int height, int size);
extern void  UArray2_free  (T *uarray2);
extern int   UArray2_width (T  uarray2);
extern int   UArray2_height(T  uarray2);
extern int   UArray2_size  (T  uarray2);
extern void *UArray2_at    (T  uarray2, int row, int col);
extern void  UArray2_map_row_major(T uarray2, void apply(int row, int col, 
                                   T uarray2, void *p1, void *p2), void *cl);
extern void  UArray2_map_col_major(T uarray2, void apply(int row, int col, 
                                   T uarray2, void *p1, void *p2), void *cl);
                                   
#undef T
#endif