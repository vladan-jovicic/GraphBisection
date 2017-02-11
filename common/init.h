#ifndef PROJECT_INIT_H
#define PROJECT_INIT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <mpi.h>

void init_int_array(int **, int);
void init_float_array(float **, int);

void init_int_matrix(int **, int, int);
void init_float_matrix(float **, int, int);


#endif //PROJECT_INIT_H
