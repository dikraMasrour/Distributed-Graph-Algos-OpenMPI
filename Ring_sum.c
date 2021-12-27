#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>




int main(int argc, char *argv[]){
	int rank, sum = 0, i, wsize;

	//creation de l'anneau
	int index[4] = {2, 4, 6, 8};
        int edges[] = {3, 1, 0, 2, 1, 3, 2, 0};
	
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
	
	//le proc initiateur
	if(rank == 0 && visited == 0){
		printf("proc %d sending to %d\n", rank, neighbors[1]);
		visited = 1;
		int dest = neighbors[1];
		MPI_Send(&rank, 1, MPI_INT, dest, 0, graph_comm); //envoi vers le voisin de droit
	}

while(1){	

		
		MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
		int pj = status.MPI_SOURCE;
		
		printf("%d : received %d\n",rank, received);
		
		//initiateur visite une autre fois
		if(rank == 0 && visited == 1){
			sum = received + rank;//decrementer msg a son arrivee a l'initiateur 
			printf("proc %d : SUM is %d\n",rank, sum);
			break;	
		}
		
		if(rank != 0){
			visited = 1;
			sum = received + rank;
			MPI_Send(&sum, 1, MPI_INT, neighbors[1], 0, graph_comm); //envoi vers le voisin de droit
			}
}

return 0;
}
