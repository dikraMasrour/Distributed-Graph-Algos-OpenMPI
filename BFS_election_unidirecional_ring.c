#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void removeElementFromArray(int array[], int *length, int element)
{
    int i, pos = -1;
    for (i = 0; i < *length; i++)
    {
        if (array[i] == element)
        {
            pos = i;
            break;
        }
    }

    // If the element was found
    if (pos != -1)
    {
        for (i = pos; i < *length - 1; i++)
        {
            array[i] = array[i + 1];
        }

        (*length)--;
    }
}


/*
    Messages utilis�s (chacun son id):
        - 0: Election(pk)
        - 1: Elu(pk)
*/

void sendMessage(int msg, int id, int dest, MPI_Comm comm)
{
    int buffer[2] = {msg, id};
    MPI_Send(buffer, 2, MPI_INT, dest, 0, comm);
}


int main(int argc, char *argv[])
{
	srand(time(0));
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Cr�ation du graphe

    /*int index[5] = {1, 4, 7, 10, 12};
    int edges[] = {1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3};
    int index[8] = {2, 5, 9, 11, 14, 17, 19, 20};
    int edges[] = {1, 2, 0, 2, 4, 0, 1, 3, 4, 2, 5, 1, 2, 5, 3, 4, 6, 5, 7, 6};*/


    int index[7] = {2, 4, 6, 8, 10, 12, 14};
    int edges[] = {6, 1, 0, 2, 1, 3, 2, 4, 3, 5, 4, 6, 5, 0};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales

    int message[2];
    int maxNeighbors = 2;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited = 0; // Si ce noeud a �tait visit� ou non
    int pred = -1;   // Le pr�d�cesseur de ce noeud
    int coordinateur = -1; //l'elu dans l'anneau
    MPI_Status status;

    // A la reception de INIT() par le processus 0
    if (pi == 0)
    {
        visited = 1;
        printf("Node 0: INIT.\n");


        //envoi de ELECTION au voisin de droite seulement
        sendMessage(0, pi, neighbors[1], graph_comm);
        //printf("im node %d my neighbors are %d and %d\n",pi, neighbors[0],  neighbors[1]);
        printf("Node %d: sending %d to %d\n", pi, pi, neighbors[1]);
    }

    while (1)
    {
        MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

	//printf("im node %d my neighbors are %d and %d\n",pi, neighbors[0],  neighbors[1]);
	printf("Node %d: received elec %d from %d\n\n", pi, message[1], pj);
        if (message[0] == 0) // A la reception de ELECTION(pk) par pi depuis pj
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (pi < message[1])
            {
                visited = 1;
                printf("Node %d: sending %d to %d\n", pi, message[1], neighbors[1]);

                //envoi de ELECTION au voisin de droite seulement
                sendMessage(0, message[1], neighbors[1], graph_comm);
            }
            else
            {
                if((pi > message[1]) && (!visited) ){
                    visited = 1;

                    //envoi de ELECTION au voisin de droite seulement
                    sendMessage(0, pi, neighbors[1], graph_comm);

                }
                else if ((pi == message[1]) || (visited)){

                    //envoi de ELU au voisin de droite seulement
                    sendMessage(1,  message[1], neighbors[1], graph_comm);
                }
            }
        }

        // A la reception de ELU(pk) par pi depuis pj

        if (message[0] == 1)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);

            if ((pi != message[1]) || (visited))
            {
                coordinateur = message[1];
                visited = 0;
                //envoi de ELU au voisin de droite seulement
                sendMessage(1, coordinateur, neighbors[1], graph_comm);
            }
            else 
            {
                printf("Anneau boucle. Coordinateur est %d\n", coordinateur);
                break;
            }
            
        }
    }

    MPI_Finalize();
    return 0;
}
