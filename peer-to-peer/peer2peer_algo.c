#include "peer2peer_algo.h"

void peer2peer_algo(int num_vertices, int *adj, int *adj_begin, int *part, int max_iters, double epsilon) {

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
    int *recv_count, **recv_idx, *send_count, **send_idx;
    int **recv_part, **send_part;
    MPI_Request *my_requests;

    // compute number of elements assigned to last proc
    if (procRank == numProcs - 1) {
        my_size = num_vertices - procRank * my_size;
    }

    /************ALLOCATION AND INITIALIZATION********************/

    // init recv_count - number of vertices received from each proc
    recv_count = (int *) malloc(sizeof(int) * numProcs);
    init_recv_count(recv_count, adj_begin, adj, start_pos, my_size, numProcs, num_vertices);

    // init recv_idx - indices of vertices received from each proc
    recv_idx = (int **) malloc(sizeof(int *) * numProcs);
    for (i = 0; i < numProcs; i++)
        recv_idx[i] = (int *) malloc(sizeof(int) * recv_count[i]);
    init_recv_idx(recv_idx, adj_begin, adj, start_pos, my_size, numProcs, num_vertices);

    // compute send_count - number of vertices sending to each proc
    send_count = (int *) malloc(sizeof(int) * numProcs);
    compute_send_count(send_count, recv_count, numProcs);

    // compute send_idx - indices of vertices sending to each proc
    send_idx = (int **) malloc(sizeof(int *) * numProcs);
    for (i = 0; i < numProcs; i++)
        send_idx[i] = (int *) malloc(sizeof(int) * send_count[i]);
    compute_send_idx(send_idx, send_count, recv_count, recv_idx, numProcs);

    // init buffers for sending and receiving labels of needed vertices
    recv_part = (int **) malloc(sizeof(int *) * numProcs);
    send_part = (int **) malloc(sizeof(int *) * numProcs);

    for (i = 0; i < numProcs; i++) {
        recv_part[i] = (int *) malloc(sizeof(int) * recv_count[i]);
        send_part[i] = (int *) malloc(sizeof(int) * send_count[i]);
    }

    //init requests array
    my_requests = (MPI_Request *)malloc(sizeof(MPI_Request) * 2 * numProcs);

    /*******************ALGORITHM******************/
    for (iter_num = 0; iter_num < max_iters; iter_num++) {
        /*****************UPDATE********************/

        one_step_iteration(num_vertices, part, start_pos, my_size, adj, adj_begin, el_threshold);

        /**************COMMUNICATION****************/

        // MPI_Request my_request;
        // MPI_Status my_status;
        for (i = 0; i < numProcs; i++) { // non-blocking send
            for (j = 0; j < send_count[i]; j++) { // compute labels of vertices that I send
                send_part[i][j] = part[send_idx[i][j]];
            }

            MPI_Isend(send_part[i], send_count[i], MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[i]);
        }

        for (i = 0; i < numProcs; i++) {
            MPI_Irecv(recv_part[i], recv_count[i], MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[numProcs + i]);
        }

        // I need to wait only recv requets but i do not want to risk
        MPI_Waitall(2 * numProcs, my_requests, MPI_STATUS_IGNORE);

        for (i = 0; i < numProcs; i++) {
            for (j = 0; j < recv_count[i]; j++) {
                part[recv_idx[i][j]] = recv_part[i][j];
            }
        }

        /***************PRINT*********************/

        print_cut_size_imbalance(num_vertices, adj_begin, adj, part);
    }
}

/** Initializes recv_count array which contains the number of elements received
 * from each processor
 * @param recv_count - a pointer to array recv_count
 * @param adjBeg - a pointer to array describing positions of neighbors
 * @param adj - a pointer to array which contains neighbors
 * @param start_pos -
 * @param my_size - size of local part assigned to processor which invokes method
 * @param numProcs - number of processors in communicator
 * @param N - number of vertices
 */
void init_recv_count(int *recv_count, int *adjBeg, int *adj, int start_pos, int my_size, int numProcs, int N) {
    int i, j, proc_owner;
    int size_per_proc = (N + numProcs - 1) / numProcs;
    memset(recv_count, 0, sizeof(int) * numProcs);
    bool *added_vertices = (bool *)malloc(N); // an array to store already added vtcs
    memset(added_vertices, 0, N);

    for (i = start_pos; i < start_pos + my_size; i++) { // for every vertex belonging to me
        for (j = adjBeg[i]; j < adjBeg[i + 1]; j++) { // for each neighbor of my current vertex
            if (added_vertices[adj[j]]) {
                continue;
            }
            added_vertices[adj[j]] = true;
            proc_owner = (int) floor((double) adj[j] / size_per_proc);
            recv_count[proc_owner] += 1;
        }
    }
}

/**
 * @param recv_idx - a pointer to an array of pointers that should be init
 * @param adjBeg - a pointer to array describing positions of neighbors
 * @param adj - a pointer to array which contains neighbors
 * @param start_pos
 * @param my_size - size of local part assigned to processor which invokes method
 * @param numProcs - number of processors in communicator
 * @param N - number of vertices
 */
void init_recv_idx(int **recv_idx, int *adjBeg, int *adj, int start_pos, int my_size, int numProcs, int N) {
    // create an array which contains the current number of elts in recv_idx[i];
    int k, j, owner;
    int *el_counter = (int *) malloc(sizeof(int) * numProcs);

    int size_per_proc = (N + numProcs - 1) / numProcs;
    memset(el_counter, 0, sizeof(int) * numProcs);
    bool *added_vertices = (bool *)malloc(N);
    memset(added_vertices, 0, N);

    for (k = start_pos; k < start_pos + my_size; k++) { // for each vertex belonging to me
        for (j = adjBeg[k]; j < adjBeg[k + 1]; j++) { // for each my neighbor
            if (added_vertices[adj[j]]) {
                continue;
            }

            added_vertices[adj[j]] = true;
            owner = (int) floor((double) adj[j] / size_per_proc);
            recv_idx[owner][el_counter[owner]] = adj[j];
            el_counter[owner] += 1;

        }
    }
}

/** Initializes send_count array which contains number of elements to send to each proc
 * @param send_count - a pointer to array that should be init
 * @param recv_count - a pointer to array describing num of elts that invoking proc receives from others
 * @param numProcs - number of processors in communicator
 */
void compute_send_count(int *send_count, int *recv_count, int numProcs) {
    int i;
    MPI_Request *my_requests = (MPI_Request *) malloc(sizeof(MPI_Request) * 2 * numProcs);
    // MPI_Status *my_statuses = (MPI_Status *)malloc(sizeof(MPI_Status) * numProcs);

    for (i = 0; i < numProcs; i++) {
        MPI_Isend(recv_count + i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[i]);
        MPI_Irecv(send_count + i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[numProcs + i]);
    }
    // I need to wait only recv requets but i do not want to risk
    MPI_Waitall(2 * numProcs, my_requests, MPI_STATUS_IGNORE);
}

/** Initializes send_idx array which contains vertices that should be sent to each proc
 * @param send_idx - a pointer to array of pointers that should be init
 * @param send_count - a pointer to array describing num of elements that proc sends to each other proc
 * @param recv_count - a pointer to array describing num of elements that proc receives from others
 * @param recv_idx - a pointer to array of pointers containing vertices that proc receives from others
 * @param numProcs - number of processors in communicator
 */
void compute_send_idx(int **send_idx, int *send_count, int *recv_count, int **recv_idx, int numProcs) {
    int i, index;
    MPI_Request *my_requests = (MPI_Request *) malloc(sizeof(MPI_Request) * 2 * numProcs);
    // MPI_Status *my_statuses = (MPI_Status *)malloc(sizeof(MPI_Status) * numProcs);

    for (i = 0; i < numProcs; i++) {
        MPI_Isend(recv_idx[i], recv_count[i], MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[i]);
        MPI_Irecv(send_idx[i], send_count[i], MPI_INT, i, 0, MPI_COMM_WORLD, &my_requests[numProcs + i]);
        //MPI_Wait(&my_request, &my_status);
    }
    // I need to wait only recv requets but i do not want to risk
    MPI_Waitall(2 * numProcs, my_requests, MPI_STATUS_IGNORE);
}