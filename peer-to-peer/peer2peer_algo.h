#ifndef PROJECT_PEER2PEER_ALGO_H
#define PROJECT_PEER2PEER_ALGO_H

#include <math.h>
#include "../common/init.h"
#include "../common/graph_funcs.h"
#include "../common/cut_funcs.h"
#include "../all-to-all/all2all_algo.h"

void init_recv_count(int *, int *, int *, int, int, int, int);

void init_recv_idx(int **, int *, int *, int, int, int, int);

void compute_send_count(int *, int *, int);

void compute_send_idx(int **, int *, int *, int **, int);

void peer2peer_algo(int, int *, int *, int *, int, double);

#endif //PROJECT_PEER2PEER_ALGO_H
