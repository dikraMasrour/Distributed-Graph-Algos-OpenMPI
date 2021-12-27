// Propagation contrôlée

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Fonction pour supprimer un élément d'un tableau
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
    Messages utilisés (chacun son id):
        - 0: Traverse()
        - 1: Retour()
*/
// id : message id
/*void sendMessage(int id, int dest, MPI_Comm comm)
{
    int buffer[] = {id};
    MPI_Send(buffer, 1, MPI_INT, dest, 0, comm);
}*/


void sendMessage(int msg[2], int dest, MPI_Comm comm)
{
    int* buffer = msg;
    MPI_Send(buffer, 2, MPI_INT, dest, 0, comm);
}


int main(int argc, char *argv[])
{
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Création du graphe

    int index[5] = {1, 4, 7, 10, 12};
    int edges[] = {1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales
    int count = 1;
    int message[2];
    int maxNeighbors = 3;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited = 0; // Si ce noeud a était visité ou non
    int pred = -1;   // Le prédécesseur de ce noeud
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        visited = 1;
        printf("Node 0: INIT.\n");
        for (i = 0; i < neighborsCount; i++)
        {
      		int msg_temp[2] = {0,-1};
      		//printf("At rank %d , count is %d\n", pi, count);
            sendMessage(msg_temp, neighbors[i], graph_comm);
        }
    }

    while (1)
    {
        MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;
        // 1st arg = message id ; 2nd arg = count
        

        // A la reception de Traverse() par pi depuis pj

        if (message[0] == 0)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (!visited)
            {
                visited = 1;
                pred = pj;
                printf("Node %d: My predecessor is %d\n", pi, pred);
                if (neighborsCount == 0)
                {
                    int msg_temp[] = {1,count}; //envoi de retour au pred avec count = 1
                    sendMessage(msg_temp, pred, graph_comm);

                    // We can safely break the while in this case
                    break;
                }
                else
                {
                    for (i = 0; i < neighborsCount; i++)
                    {
                    	int msg_temp[] = {0,-1}; //envoi de traverse aux voisins
                        sendMessage(msg_temp, neighbors[i], graph_comm);
                    }
                }
            }
            else
            {
            	int msg_temp[2] = {1, 0}; //déjà visité, envoi de retour avec count = 0
                sendMessage(msg_temp, pj, graph_comm);
            }
        }

        // A la reception de Retour() par pi depuis pj

        if (message[0] == 1)
        {
		count = count + message[1];
            	removeElementFromArray(neighbors, &neighborsCount, pj);
            if (neighborsCount == 0)
            {
            
         	   
                if (pred == -1)
                {
                    printf("There are %d nodes in the graph\n", count);
                    printf("Node %d: END.\n", pi);
                    
                }
                else
                {
                    int msg_temp[2] = {1,count};
                    sendMessage(msg_temp, pred, graph_comm);
                }

                // We can safely break the while in both cases
                break;
            }
        }
    }

    MPI_Finalize();
    return 0;
}
