#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char *argv[]){

	srand(time(0));
	int rank, sum = 0, i, j, wsize;

	//creation de l'anneau
	int index[4] = {1, 2, 5, 6};
        int edges[] = {2, 2, 0, 1, 3, 2};
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &wsize);
	MPI_Comm graph_comm;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    	MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
	
	//var locales
	int received = -1;
	int visited = 0;
	int maxNeighbors = 2;
	int neighborsCount;
	int neighbors[maxNeighbors]; // Liste des voisins
	
	
	MPI_Graph_neighbors_count(graph_comm, rank, &neighborsCount);
	MPI_Graph_neighbors(graph_comm, rank, maxNeighbors, neighbors);
	MPI_Status status;
	
	
	//proc maitre
	if(rank == 2){
		for(i = 0; i < neighborsCount; i++){
			j = 0 + rand() % 10;
			MPI_Send(&j, 1, MPI_INT, neighbors[i], 0, graph_comm);
			printf("sent to %d\n", neighbors[i]);
		}
	}
	
	while(rank != 2){
	
		MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
		int pj = status.MPI_SOURCE;
		
		
		printf("Proc %d : received %d from %d\n", rank, received, pj);
		break;
	
	
	}
	
	
	MPI_Finalize();
	return 0;
}
