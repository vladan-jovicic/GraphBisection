#include "common/init.h"
#include "common/read.h"
#include "common/cut_funcs.h"
#include "sequential/sequential_gblp.h"
#include "all-to-all/all2all_algo.h"
#include "peer-to-peer/peer2peer_algo.h"

void print_usage() {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
        printf("Usage: mpirun -np [num-procs] ./GraphBisection [graph-file-name] [bisect-algo-name] [max-iters] [epsilon] [random-seed]\n"
                       "Arguments:\n"
                       "\t[num-procs]: The number of MPI ranks to be used. For the sequential algorithm it has to be 1.\n"
                       "\t[graph-file-name]: Name of the graph file.\n"
                       "\t[bisect-algo-name]: Name of the graph bisection algorithm. There are three possibilities:\n"
                       "\t\tbisect-seq: Sequential bisection algorithm.\n"
                       "\t\tbisect-a2a: Parallel bisection algorithm using all-to-all communication routines.\n"
                       "\t\tbisect-p2p: Parallel bisection algorithm using point-to-point communication routines.\n"
                       "\t[max-iters]: The number of iterations for the label propagation to run.\n"
                       "\t[epsilon]: Maximum allowed imbalance ratio for the label propagation.\n"
                       "\t[seed]: Random seed used to initialize the part array. This parameter is optional, and you can use it to test the correctness of your algorithm. If not specified, it is set to time(0).\n"
        );
    }
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    if (argc < 5) {
        print_usage();
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int proc_rank, max_iters, num_vertices, num_edges, seed;
    int *part, *adj_begin, *adj;
    char *algo_to_use;
    double epsilon;

    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
	algo_to_use = argv[2];
    max_iters = atoi(argv[3]);
    epsilon = atof(argv[4]);

    if (argc == 6) {
        seed = atoi(argv[5]);
    } else {
        seed = time(0);
    }
    read_graph(argv[1], &num_vertices, &num_edges, &adj_begin, &adj);
	printf("%d %d\n", num_vertices, num_edges);
    random_cut(&part, num_vertices, seed);
    double startTime = MPI_Wtime();
    if (strcmp(algo_to_use, "bisect-seq") == 0) {
        sequential_algo(num_vertices, adj, adj_begin, part, max_iters, epsilon);
    } else if(strcmp(algo_to_use, "bisect-a2a") == 0) {
        all2all_algo(num_vertices, adj, adj_begin, part, max_iters, epsilon);
    } else if(strcmp(algo_to_use, "bisect-p2p") == 0) {
        peer2peer_algo(num_vertices, adj, adj_begin, part, max_iters, epsilon);
    } else {
	}

    if (proc_rank == 0) {
        printf("bisectGraph took %e seconds.\n", MPI_Wtime() - startTime);
    }
    MPI_Finalize();
    return 0;
}
