#include "init.h"
#include "read.h"


void read_graph(char *file_name, int *num_vertices, int *num_edges, int **adj_begin, int **adj) {
    FILE *file = fopen(file_name, "r");

    if (file == NULL) {
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        if (rank == 0) {
            printf("ERROR: Unable to open the file %s.\n", file_name);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

	int n_edges, n_vertices;
    fscanf(file, "%d %d", &n_vertices, &n_edges);
	*num_vertices = n_vertices;
	*num_edges = n_edges;
    int *edge_left, *edge_right, i, *p_adj_begin, *p_adj;

	edge_left = (int *) malloc(*num_edges * sizeof(int));
	edge_right = (int *) malloc(*num_edges * sizeof(int));
	*adj_begin = p_adj_begin = (int *) malloc((*num_vertices + 1) * sizeof(int));
	*adj = p_adj = (int *) malloc(sizeof(int) * ( 2 * *num_edges));

    // compute degrees
    for (i = 0; i < *num_edges; i++) {
        fscanf(file, " %d %d", edge_left + i, edge_right + i);
		p_adj_begin[edge_left[i]] ++;
		p_adj_begin[edge_right[i]] ++;
    }

    // compute actual positions
    for (i = 1; i <= *num_vertices; i++) {
		p_adj_begin[i] += p_adj_begin[i - 1];
    }

    for (i = 0; i < *num_edges; i++) {
		p_adj[--p_adj_begin[edge_left[i]]] = edge_right[i];

		p_adj[--p_adj_begin[edge_right[i]]] = edge_left[i];
    }

    free(edge_left);
    free(edge_right);
    fclose(file);
}