#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


int main(int argc, char * argv[]) 
{
    int wsize;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // The number of nodes is the number of proccess
    int nnodes = wsize;
    int index[5] = { 1, 4, 7, 10, 12 }; //cumulative degrees !!
    int edges[] = { 1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3 };

    // Create a new graph comm
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);

    // Some testing
    int i, realRank, neighborsCount;
    // #1 Number of neighbors per node
    MPI_Comm_rank(graph_comm, &realRank);
    MPI_Graph_neighbors_count(graph_comm, realRank, &neighborsCount);
    printf("Rank %d: Neighbors count = %d\n", realRank, neighborsCount);
    // #2 Neighbors of each node
    int neighbors[5];
    MPI_Graph_neighbors(graph_comm, realRank, 5, neighbors);
    for (i = 0; i < neighborsCount; i++)
    {
        printf("Rank %d: Neighbor %d\n", realRank, neighbors[i]);
    }
    printf("\n");

    MPI_Finalize();
    return 0;
}

