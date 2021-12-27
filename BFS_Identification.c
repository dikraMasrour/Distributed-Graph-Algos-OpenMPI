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

void addElementToArray(int array[], int *length, int element)
{
    int i, j = 0;
    
    array[*length] = element;
    ++(*length) ;
}

/*
    Messages utilisés (chacun son id):
        - 0: Traverse()
        - 1: Retour()
        - 2: Identification()
        - 3: Terminer()
*/


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
    int filsCount;
    int fils[maxNeighbors];
    int tailles[wsize];
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited = 0; // Si ce noeud a était visité ou non
    int pred = -1;   // Le prédécesseur de ce noeud
    int phase = 1;
    int id = 0;
    int cid;
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        visited = 1;
        printf("Node 0: INIT.\n");
        for (i = 0; i < neighborsCount; i++)
        {
      		int msg_temp[2] = {0,-1};
            sendMessage(msg_temp, neighbors[i], graph_comm);
        }
    }

    while (1)
    {
        MPI_Recv(&message, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

        // 1st arg = message id ; 2nd arg = count or id
        // A la reception de Traverse() par pi depuis pj

        if (message[0] == 0)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (!visited)
            {
                visited = 1;
                pred = pj;
                printf("Node %d: My predecessor is %d\n\n", pi, pred);
                if (neighborsCount == 0)
                {
                    int msg_temp[] = {1,count}; //envoi de retour au pred avec count = 1
                    sendMessage(msg_temp, pred, graph_comm);

                    phase = 2; // mise en attente de la phase 2
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


        // A la reception de Retour(c) par pi depuis pj

        if (message[0] == 1) 
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if(message[1] > 0){
		 count = count + message[1]; // mise à jour du compteur
                tailles[pj] = message[1]; //  taille de la sous arb du fils pj
                addElementToArray(fils, &filsCount, pj);
            }

            if (neighborsCount == 0)
            {	
                if (pred == -1)
                {
                    //initier la phase 2
                    cid = id + 1;
                    for (int i = 0; i < filsCount; i++)
                    {
                        int temp[2] = {2, cid}; //envoi d'un msg d'identification : enclenchement de la phase 2
                        sendMessage(temp, fils[i], graph_comm);
                        cid += tailles[fils[i]];
                    }
                }
                else
                {
                    int msg_temp[2] = {1,count};
                    sendMessage(msg_temp, pred, graph_comm);
                }

                phase = 2;
            }
        }

        //A la reception de IDENTIFICATION(n) 

        if(message[0] == 2 && phase == 2)
        {
            id = message[1];
            printf("Node %d: Received ID %d from %d\n", pi, id, pj);
            if (filsCount != 0)
            {
                cid = id + 1;
                for (int i = 0; i < filsCount; i++)
                {
                    int temp[2] = {2, cid}; //envoi d'un msg d'identification : enclenchement de la phase 2
                    sendMessage(temp, fils[i], graph_comm);
                    cid += tailles[fils[i]];
                }
            }
            else
            {
                int temp[2] = {3, -1}; 
                sendMessage(temp, pred, graph_comm); //envoyer terminer au predecesseur
                break;
            }
            
        }


        //A la reception de TERMINER() de Pi depuis Pj
        if (message[0] == 3 && phase == 2)
        {
        //printf("im %d i received %d de %d\n", pi, message[0], pj);
            removeElementFromArray(fils, &filsCount, pj);
            if(filsCount == 0){
                if (pi == 0)
                {
                    printf("Node %d: END \n", pi);
                }
                else{
                    int temp[2] = {3, -1}; 
                    sendMessage(temp, pred, graph_comm); //envoyer terminer au predecesseur
                }
                break;
            }
            
        }
        
    }

    MPI_Finalize();
    return 0;
}
