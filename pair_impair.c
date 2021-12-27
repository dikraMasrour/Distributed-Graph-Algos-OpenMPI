#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char *argv[]){

	int rank;
	
	// Initialisation
	MPI_Init(&argc, &argv);
	
	// Quel est mon rang ?
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(rank % 2 == 0){
		printf("Je suis un process PAIR de rang %d\n", rank);
	}
	else if (rank % 2 == 1){
		printf("Je suis un process IMPAIR de rang %d\n", rank);
	}
	
	MPI_Finalize();
	return 0;
}
