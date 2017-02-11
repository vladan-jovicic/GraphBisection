#include "../common/init.h"
#include "../common/graph_funcs.h"
#include "../common/cut_funcs.h"
#include "sequential_gblp.h"

void sequential_algo(int num_vertices, int *adj, int *adj_begin, int *part, int max_iters, double epsilon) {
    int procRank, numProcs;
    print_cut_size_imbalance(num_vertices, adj_begin, adj, part);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    int i, j, k, iter_num = 0;
    int nb_other_part = 0, nb_part = 0, cmp_part = 0;
    double el_threshold = ((double) num_vertices / 2.0) * epsilon;
    int neigh_curr_part = 0, neigh_other_part = 0;


    if (procRank == 0) {
        if (numProcs != 1) {
            printf("ERROR: Sequential algorithm is ran with %d MPI processes.\n", numProcs);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    // compute number of elements in initial configuration
    nb_part = nb_other_part = 0;
    for (i = 0; i < num_vertices; i++) {
        nb_part += 1 - part[i];
        nb_other_part += part[i];
    }

    /*********************ALGORITHM**********************/
    for (iter_num = 0; iter_num < max_iters; iter_num++) {
        for (i = 0; i < num_vertices; i++) {
            part[i] = 1 - part[i];

            // cmp_part is number of elements in part where we want to move vertex i
            cmp_part = (part[i] == 0) ? nb_part : nb_other_part;
            if (cmp_part > el_threshold) {
                part[i] = 1 - part[i];
                continue;
            }

            // compute the number of neighbors in each part
            neigh_curr_part = neigh_other_part = 0;
            // check what is other part
            if (part[i] == 0) {
                nb_of_neighbours(i, adj, adj_begin, part, &neigh_curr_part, &neigh_other_part);
            } else {
                nb_of_neighbours(i, adj, adj_begin, part, &neigh_other_part, &neigh_curr_part);
            }

            if (neigh_other_part < neigh_curr_part) {
                nb_other_part += (part[i] + (1 - part[i]) * (-1));
                nb_part += (1 - part[i] + part[i] * (-1));
            } else {
                part[i] = 1 - part[i]; // I can not move it, reverse it
            }
        }
        print_cut_size_imbalance(num_vertices, adj_begin, adj, part);
    }
}

