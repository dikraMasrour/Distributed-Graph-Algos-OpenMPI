#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

int main(int argc, char *argv[]){
	srand(time(0));
	int rank;
	float* valeurs, vals;
	float start, end;
	// Initialisation de la biblioth`que MPI
	MPI_Init(&argc, &argv);
	
	// Quel est mon rang ?
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	if(rank == 0){
		for (int i = 0; i < 1000; i++){
			valeurs[i] = rand() / (float) (RAND_MAX + 1.);
		}
		start = MPI_Wtime();
		MPI_Send(valeurs, 1000, MPI_FLOAT, 1, 1, MPI_COMM_WORLD);
		MPI_Recv(valeurs, 1000, MPI_FLOAT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		end = MPI_Wtime();
		printf("I am process of rank %d, I sent and received values from Process0 in %f seconds\n", rank, end-start);
	}
	else if (rank == 1){
	
		MPI_Recv(valeurs, 1000, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Proc %d : Modifying...\n",	 rank);
		for (int i = 0; i < 1000; i++){
			valeurs[i]++;
		}
		MPI_Send(valeurs, 1000, MPI_FLOAT, 0, 1, MPI_COMM_WORLD);
	}
	MPI_Finalize();
return 0;
}
