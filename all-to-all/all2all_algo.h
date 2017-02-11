#ifndef PROJECT_ALL2ALL_ALGO_H
#define PROJECT_ALL2ALL_ALGO_H

#include "../common/init.h"
#include "../common/graph_funcs.h"
#include "../common/cut_funcs.h"

void init_recv_buffers(int , int , int *, int *);

int compute_size_of_partitions(int *, int, int);

void one_step_iteration(int, int *, int, int, int *, int *, double);

void all2all_algo(int, int *, int *, int *, int, double);

#endif //PROJECT_ALL2ALL_ALGO_H
