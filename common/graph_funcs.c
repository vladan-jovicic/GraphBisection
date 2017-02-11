#include "graph_funcs.h"


/** Computes the number of neighbors in each part
 * @param i - vertex for which we compute
 * @param part - pointer to the first part
 * @param other_part - pointer to the second part
 * @param neigh_part - pointer to the number of neighbours in the first part
 * @param neigh_other_part - pointer to the number of neighbours in the second part
 */
void nb_of_neighbours(int i,int *adj,int *adjBeg,int *part,int *neigh_part_zero,int *neigh_part_one) {
    int j;
    *neigh_part_zero = 0;
    *neigh_part_one = 0;
    for (j = adjBeg[i]; j < adjBeg[i + 1]; j++) {
        *neigh_part_one = *neigh_part_one + part[adj[j]];
        *neigh_part_zero = *neigh_part_zero + 1 - part[adj[j]];
    }
}