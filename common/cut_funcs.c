#include "cut_funcs.h"
#include "init.h"

void random_cut(int **part, int num_vertices, int seed) {
	int *p_part;
    *part = p_part = (int *) malloc(sizeof(int) * num_vertices);

    int rank, i;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // only one processor will init and broadcast to all others
    // could also paralelize this with scatter and gather
    if (rank == 0) {
        srand(seed);
        for (i = 0; i < num_vertices; i++) {
            p_part[i] = rand() % 2;
        }
    }

    // broadcast to everyone else
    MPI_Bcast(p_part, num_vertices, MPI_INT, 0, MPI_COMM_WORLD);
}

void print_cut_size_imbalance(int num_vertices, int *adj_begin, int *adj, int *part) {
	int cutSize = 0;
	int numProcs, procRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &procRank);
	MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
	int vtxPerProc = (num_vertices + numProcs - 1) / numProcs;
	int vtxBeg = procRank * vtxPerProc;
	int vtxEnd = (procRank == numProcs - 1) ? num_vertices : (procRank + 1) * vtxPerProc;
	int i;
	int partZeroCount = 0;
	for (i = vtxBeg; i < vtxEnd; i++) { // Make a pass over the local vertices, compute the local cutsize
		int j;
		if (part[i] == 0) { partZeroCount++; }
		for (j = adj_begin[i]; j < adj_begin[i + 1]; j++) {
			if (part[adj[j]] != part[i]) { // A cut edge is found
				cutSize++;
			}
		}
	}
	MPI_Allreduce(MPI_IN_PLACE, &cutSize, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD); // Sum all cut edges
	MPI_Allreduce(MPI_IN_PLACE, &partZeroCount, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD); // Sum all part zero counts
	cutSize /= 2; // Note that every cut edge is counted exactly twice.
	double imbal;
	if (2 * partZeroCount >= num_vertices) { // There are more 0s
		imbal = (2.0 * partZeroCount) / num_vertices;
	} else { // There are more 1s
		imbal = (2.0 * (num_vertices - partZeroCount)) / num_vertices;
	}
	if (procRank == 0) {
		printf("cutsize: %d imbal: %.3e\n", cutSize, imbal);
	}
}