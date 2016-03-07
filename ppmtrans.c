/*
 * ppmtrans.c
 * Bill Yung & Dani Kupfer | Comp 40 | HW 3
 * 
 * This is an implementation of ppmtrans, which 
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

const int VERTICAL   = 1;
const int HORIZONTAL = 2;

static FILE *open_file(int argc, char *argv[], int i);
static Pnm_ppm transform_image(int rotation, A2Methods_T methods, 
                               Pnm_ppm pixmap, A2Methods_mapfun *map);
static Pnm_ppm transform_diff_dim(int rotation, A2Methods_T methods, 
                                  Pnm_ppm pixmap, A2Methods_mapfun *map, 
                                  A2Methods_UArray2 orig);
static Pnm_ppm transform_same_dim(int rotation, A2Methods_T methods, 
                                  Pnm_ppm pixmap, A2Methods_mapfun *map, 
                                  A2Methods_UArray2 orig);
static void rotation_90(int i, int j, A2Methods_UArray2 array2, 
                        A2Methods_Object *ptr, void *cl);
static void rotation_180(int i, int j, A2Methods_UArray2 array2, 
                         A2Methods_Object *ptr, void *cl);
static void rotation_270(int i, int j, A2Methods_UArray2 array2, 
                         A2Methods_Object *ptr, void *cl);
static void flip_vert(int i, int j, A2Methods_UArray2 array2, 
                      A2Methods_Object *ptr, void *cl);
void flip_hor(int i, int j, A2Methods_UArray2 array2, 
              A2Methods_Object *ptr, void *cl);
static void close_file(FILE *src);

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                "[-{row,col,block}-major] [filename]\n",
                progname);
        exit(1);
}

int main(int argc, char *argv[]) 
{
        int i;
        int rotation = 0;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

#define SET_METHODS(METHODS, MAP, WHAT) do {                            \
                methods = (METHODS);                                    \
                assert(methods != NULL);                                \
                map = methods->MAP;                                     \
                if (map == NULL) {                                      \
                        fprintf(stderr, "%s does not support "          \
                                        WHAT "mapping\n",               \
                                argv[0]);                               \
                        exit(1);                                        \
                }                                                       \
        } while (0)


        for (i = 1; i < argc; i++) {
                if (!strcmp(argv[i], "-row-major")) {
                        SET_METHODS(uarray2_methods_plain, map_row_major,
                                    "row-major");
                } else if (!strcmp(argv[i], "-col-major")) {
                        SET_METHODS(uarray2_methods_plain, map_col_major,
                                    "column-major");
                } else if (!strcmp(argv[i], "-block-major")) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (!strcmp(argv[i], "-rotate")) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90
                              || rotation == 180 || rotation == 270)) {
                                fprintf(stderr, "Rotation must be "
                                        "0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (!strcmp(argv[i], "-flip")) {
                        if (!(i + 1 < argc)) {      /* no flip value */
                                usage(argv[0]);
                        }
                        if (!strcmp(argv[++i], "vertical"))
                                rotation = VERTICAL;
                        else if (!strcmp(argv[i], "horizontal"))
                                rotation = HORIZONTAL;
                        else {
                                fprintf(stderr, "Flipping must be either "
                                        "vertical or horizontal\n");
                                usage(argv[0]);
                        }
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }
        
        FILE *src      = open_file(argc, argv, i);
        Pnm_ppm pixmap = Pnm_ppmread(src, methods);
        pixmap         = transform_image(rotation, methods, pixmap, map);
                         Pnm_ppmwrite(stdout, pixmap);
                         close_file(src);
        
        Pnm_ppmfree(&pixmap);
}

/*opens the file if not provided from standard in */
FILE *open_file(int argc, char *argv[], int i)
{  
        /* reading from input programming idiom */
        FILE *src = stdin;
        if (argc > i) {
                FILE *fp = fopen(argv[i], "r");
                assert(fp != NULL);
                src = fp;
        }
        return src;
}

/*
 * Looks at user's specifications for transforming the image
 * Calls the correct function to transform the image correctly
 */
Pnm_ppm transform_image(int rotation, A2Methods_T methods, Pnm_ppm pixmap, 
                     A2Methods_mapfun *map) 
{
        A2Methods_UArray2 orig = pixmap->pixels;
        
        if (rotation == 0) {
                return pixmap;
        } else if (rotation == 90 || rotation == 270) {
                return transform_diff_dim(rotation, methods, pixmap, map, 
                                          orig);

        } else if (rotation == 180 || rotation == VERTICAL || 
                   rotation == HORIZONTAL) {             
                return transform_same_dim(rotation, methods, pixmap, map, 
                                          orig);
        } 
        
        return pixmap;
}

/*
 * Helper function that rotates the image
 * This function transforms the image that do not need new dimensions after
 * rotation.
 * (Rotate 90, rotate 270)
 */  
Pnm_ppm transform_diff_dim(int rotation, A2Methods_T methods, Pnm_ppm pixmap, 
                     A2Methods_mapfun *map, A2Methods_UArray2 orig)
{
        CPUTime_T timer = CPUTime_New();
        double time = 0;
        
        pixmap->pixels = methods->new_with_blocksize(methods->height(orig), 
                                                     methods->width(orig), 
                                                     methods->size(orig),
                                                     methods->blocksize(orig));
        if (rotation == 90) {
                CPUTime_Start(timer);
                map(orig, rotation_90, pixmap);
                time = CPUTime_Stop(timer);
        }
        else {
                CPUTime_Start(timer);
                map(orig, rotation_270, pixmap);
                time = CPUTime_Stop(timer);
        }

        /* Changes the height and width after rotating */
        int height = pixmap->height;
        pixmap->height = pixmap->width;
        pixmap->width = height;

        double time_per_pixel = time / (pixmap->width * pixmap->height);
        FILE *timings_file = fopen("timingfile", "a");
        fprintf(timings_file, "Image Dimensions: %u x %u\n", pixmap->width, 
                                                             pixmap->height);
        fprintf(timings_file, "Total: %.0f nanosecond\n", time);
        fprintf(timings_file, "Per pixel: %.0f nanoseconds\n", time_per_pixel);
        CPUTime_Free(&timer);
        
        methods->free(&orig);
        return pixmap;    
}

/*
 * Helper function that rotates the image
 * This function transforms the image that do not need new dimensions after
 * rotation.
 * (Rotate 180, flip vertical, flip horizontal)
 */  
Pnm_ppm transform_same_dim(int rotation, A2Methods_T methods, Pnm_ppm pixmap, 
                     A2Methods_mapfun *map, A2Methods_UArray2 orig)
{
        CPUTime_T timer = CPUTime_New();
        double time = 0;
        
        pixmap->pixels = methods->new_with_blocksize(methods->width(orig), 
                                                     methods->height(orig), 
                                                     methods->size(orig),
                                                     methods->blocksize(orig));
        if (rotation == 180) {
                CPUTime_Start(timer);
                map(orig, rotation_180, pixmap);
                time = CPUTime_Stop(timer);
        }
        else if (rotation == VERTICAL) {
                CPUTime_Start(timer);
                map(orig, flip_vert, pixmap);
                time = CPUTime_Stop(timer);
        }
        else {
                CPUTime_Start(timer);
                map(orig, flip_hor, pixmap);
                time = CPUTime_Stop(timer);
        }
        
        double time_per_pixel = time / (pixmap->width * pixmap->height);
        FILE *timings_file = fopen("timingfile", "a");
        fprintf(timings_file, "Image Dimensions: %d x %d\n", pixmap->width, 
                                                             pixmap->height);
        fprintf(timings_file, "Total: %.0f nanosecond\n", time);
        fprintf(timings_file, "Per pixel: %.0f nanoseconds\n", time_per_pixel);
        CPUTime_Free(&timer);        
        
        
        methods->free(&orig);
        return pixmap;    
}

/* Apply function to rotate image 90 degrees */
void rotation_90(int i, int j, A2Methods_UArray2 array2, A2Methods_Object *ptr, 
                  void *cl)
{
        (void)array2;

        Pnm_ppm pixmap = (Pnm_ppm)cl;  

        int new_col = pixmap->height - j - 1;
        int new_row = i;     
             
        *(Pnm_rgb)(pixmap->methods->
                        at(pixmap->pixels, new_col, new_row)) = *(Pnm_rgb)ptr;

}

/* Apply function that rotates image 180 degrees */
void rotation_180(int i, int j, A2Methods_UArray2 array2, A2Methods_Object *ptr,
                  void *cl)
{
        (void)array2;

        Pnm_ppm pixmap = (Pnm_ppm)cl;  

        int new_col = pixmap->width - i - 1;
        int new_row = pixmap->height - j - 1;
             
        *(Pnm_rgb)(pixmap->methods->
                at(pixmap->pixels, new_col, new_row)) = *(Pnm_rgb)ptr;
}

/* Apply function that rotates the image 270 degrees */
void rotation_270(int i, int j, A2Methods_UArray2 array2, A2Methods_Object *ptr, 
                  void *cl)
{
        (void)array2;

        Pnm_ppm pixmap = (Pnm_ppm)cl;  

        int new_col = j;
        int new_row = pixmap->width - i - 1;      
             
        *(Pnm_rgb)(pixmap->methods->
                at(pixmap->pixels, new_col, new_row)) = *(Pnm_rgb)ptr;
}

/* Apply function that flips the image vertically */
void flip_vert(int i, int j, A2Methods_UArray2 array2, 
               A2Methods_Object *ptr, void *cl)
{
        (void)array2;

        Pnm_ppm pixmap = (Pnm_ppm)cl;  

        int new_col = i;
        int new_row = pixmap->height - j - 1;
             
        *(Pnm_rgb)(pixmap->methods->
                at(pixmap->pixels, new_col, new_row)) = *(Pnm_rgb)ptr;
}

/* Apply function that flips the image horizontally */
void flip_hor(int i, int j, A2Methods_UArray2 array2, 
              A2Methods_Object *ptr, void *cl)
{
        (void)array2;

        Pnm_ppm pixmap = (Pnm_ppm)cl;  

        int new_col = pixmap->width - i - 1;
        int new_row = j;
        
        *(Pnm_rgb)(pixmap->methods->
                at(pixmap->pixels, new_col, new_row)) = *(Pnm_rgb)ptr;        
}

/* closes the file if it not read in from standard in */
void close_file(FILE *src)
{
        if (src != stdin) fclose(src); 
}

