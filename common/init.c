#include "init.h"


void init_int_array(int **array, int size) {
    (*array) = (int *) malloc(size * sizeof(int));

    memset(array, 0, size * sizeof(int));
}

void init_float_array(float **array, int size) {
    *array = (float *) malloc(size * sizeof(float));

    memset(array, 0, size * sizeof(float));
}

void init_int_matrix(int **array, int rows, int columns) {
    *array = (int *) malloc(rows * columns * sizeof(int));

    memset(array, 0, rows * columns * sizeof(int));
}

void init_float_matrix(float **array, int rows, int columns) {
    *array = (float *) malloc(rows * columns * sizeof(float));

    memset(array, 0, rows * columns * sizeof(float));
}

