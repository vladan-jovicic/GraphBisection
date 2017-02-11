#include "all2all_algo.h"

void all2all_algo(int num_vertices, int *adj, int *adj_begin, int *part, int max_iters, double epsilon) {

    int procRank, numProcs;
    print_cut_size_imbalance(num_vertices, adj_begin, adj, part);
    MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    int i, j, k, iter_num = 0;
    // int nb_other_part = 0, nb_part = 0, cmp_part = 0;
    double el_threshold = ((double) num_vertices / 2.0) * epsilon;
    // int neigh_curr_part = 0, neigh_other_part = 0;

    int my_size = (num_vertices + numProcs - 1) / numProcs;
    int start_pos = my_size * procRank;
    int *recv_count, *offsets;

    // compute number of elements owned by the last proc
    if (procRank == numProcs - 1) {
        my_size = num_vertices - procRank * my_size;
    }

    // initialize arrays for for MPI_Allgatherv;
    recv_count = (int *) malloc(numProcs * sizeof(int));
    offsets = (int *) malloc(numProcs * sizeof(int));
    init_recv_buffers(num_vertices, numProcs, recv_count, offsets);

    for (iter_num = 0; iter_num < max_iters; iter_num++) {
        /*********************UPDATE**********************/

        one_step_iteration(num_vertices, part, start_pos, my_size, adj, adj_begin, el_threshold);

        /*****************COMMUNICATION*******************/

        MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_INT, part, recv_count, offsets, MPI_INT, MPI_COMM_WORLD);

        print_cut_size_imbalance(num_vertices, adj_begin, adj, part);
    }
}


/** Computes the number of elements in each partition
 * @param part - array defining partitions
 * @param start_post - starting position of the processor which invokes method
 * @param my_size - size of portion which updates processor which invokes method
*/
int compute_size_of_partitions(int *part, int start_pos, int my_size) {
    int i, j;
    int nb_elements = 0;

    // compute the number of elements in local part
    for (i = start_pos; i < start_pos + my_size; i++) {
        nb_elements += (1 - part[i]);
    }

    //send locall parts, combine, and get it back
    MPI_Allreduce(MPI_IN_PLACE, &nb_elements, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    return nb_elements;
}

/** Computes one iteration
 * @param N
 * @param part
 * @param start_pos
 * @param my_size
 * @param adj
 * @param adjBeg
 * @param next_part
 * @param el_threshold
 */
void one_step_iteration(int N, int *part, int start_pos, int my_size, int *adj, int *adjBeg, double el_threshold) {
    int nb_part, nb_other_part, i, cmp_part;

    nb_part = compute_size_of_partitions(part, start_pos, my_size);
    nb_other_part = N - nb_part;

    for (i = start_pos; i < start_pos + my_size; i++) {
        part[i] = 1 - part[i];

        cmp_part = (part[i] == 0) ? nb_part : nb_other_part;
        if (cmp_part > el_threshold) {
            part[i] = 1 - part[i];
        } else {

            // compute the number of neighbors in each part
            int neigh_curr_part = 0, neigh_other_part = 0;
            // check what is other part
            if (part[i] == 0) { // I am in part 0 right now
                nb_of_neighbours(i, adj, adjBeg, part, &neigh_curr_part, &neigh_other_part);
            } else {
                nb_of_neighbours(i, adj, adjBeg, part, &neigh_other_part, &neigh_curr_part);
            }
            if (neigh_other_part > neigh_curr_part) { // it is better for me to keep my old part
                part[i] = 1 - part[i];
            }
        }
    }
}

/** Initializes recv_count for communication in bisect-a2a
 * @param N - number of vertices
 * @param numProcs - number of processors
 * @param recv_count - pointer to array to store counts
 * @param offsets - pointer to array of offsets for recv_buff
*/
void init_recv_buffers(int N, int numProcs, int *recv_count, int *offsets) {
    int i;
    int size = (N + numProcs - 1) / numProcs;

    for (i = 0; i < numProcs; i++) {
        recv_count[i] = size;
        offsets[i] = i * size;
    }

    if (N % numProcs != 0) {
        recv_count[numProcs - 1] = N - (numProcs - 1) * size;
    }

    /*// offsets[0] = 0;
    for (i = 0; i < numProcs; i++) {
        // offsets[i] = offsets[i - 1] + recv_count[i];
        offsets[i] = i*size;
    }*/
}
