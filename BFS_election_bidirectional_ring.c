#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <stdbool.h>


//Etats d'un process
typedef enum etats{
    NON_CONCERNE,
    CANDIDAT,
    BATTU,
    ELU
};

/***Dikra MASROUR****/
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
    Messages utilises (chacun son id):
        - 0: Candidature(pk, lg, lgmax)
        - 1: Reponse(pk, bool)
        - 2: Terminer(pk)
*/

void sendMessage(int msg, int id, int lgOrbool, int lgmax, int dest, MPI_Comm comm)
{
    int buffer[4] = {msg, id, lgOrbool, lgmax};
    MPI_Send(buffer, 4, MPI_INT, dest, 0, comm);
}

void faire_suivre(int msg, int id, int lgOrbool, int lgmax, int pj, int neighbors[2], MPI_Comm comm)
{
    int buffer[4] = {msg, id, lgOrbool, lgmax};

    if (pj == neighbors[0])
    {
        MPI_Send(buffer, 4, MPI_INT, neighbors[1], 0, comm);
    }
    else if (pj == neighbors[1])
    {
        MPI_Send(buffer, 4, MPI_INT, neighbors[0], 0, comm);
    }
    
}


int main(int argc, char *argv[])
{
	srand(time(0));
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Creation du graphe

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

    int message[4];
    int maxNeighbors = 2;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    enum etats etat = NON_CONCERNE;
    int nbrep = 0; //nombre de reponses attendues
    bool repOk = 1;
    int lgmax = 0;
    int lg = 0;
    int vainqueur = -1; //l'elu dans l'anneau
    bool mybool = 0;
    MPI_Status status;


        // Tous les processus sont initiateurs
       
        etat = CANDIDAT;
        lgmax = 1;
        
        //envoi double Candidature
        sendMessage(0, pi, lg, lgmax, neighbors[0], graph_comm);
        sendMessage(0, pi, lg, lgmax, neighbors[1], graph_comm);


while(1){

	MPI_Recv(&message, 4, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
	int pj = status.MPI_SOURCE;
            

            //A la reception de Reponse(pk, bool)
            if (message[0] == 1)
            {
            	mybool = message[2];
            	printf("I am %d I have received Reponse from %d\n", pi, pj);
                if (message[1] == pi)
                {
                    ++nbrep;
                    repOk = repOk && mybool;
                    //attente de 2 reponses
                    if(nbrep == 2){
		            if(repOk == 0){
		                etat = BATTU;
		            }
		            lgmax *= 2;
		            if(etat == CANDIDAT)
		            {
		                printf(" %d canditature  lgmax %d \n",pi,lgmax);

		                nbrep = 0;
		                repOk = 1;
		                
	                        //envoi double Candidature
				sendMessage(0, pi, lg, lgmax, neighbors[0], graph_comm);
				sendMessage(0, pi, lg, lgmax, neighbors[1], graph_comm);
		            }

                }
                }
                else faire_suivre(1, message[1], message[2], -1, pj, neighbors, graph_comm);
                
            }
            
            /*if (nbrep == 2)
            {
                if (!repOk) etat = BATTU;
                lgmax = lgmax * 2;
                printf("02 : I am %d i was defeated and my status is %c\n", pi, etat);
                
            }*/

            
    //A la reception de Candidature(pk, lg, lgmax)
    if (message[0] == 0)
    {
        printf("I am %d I have received Candidature from %d\n", pi, pj);
        if(message[1] - pi > 0){
            etat = BATTU;
            printf("NODE %d : i was defeated and my status is %c\n", pi, etat);
            
            if (message[2] < message[3]){
            	lg = message[2] + 1;
            	lgmax = message[3];
                faire_suivre(0, message[1], lg, lgmax, pj, neighbors, graph_comm);
            }
            else{
            	
            	sendMessage(1, message[1], 1, -1, pj, graph_comm); //sending Reponse()
            }
                
        }
        else if (message[1] - pi < 0)
        {
            sendMessage(1, message[1], 0, -1, pj, graph_comm);
        }
        else{
		if (etat != ELU )
		{
		    etat = ELU;
		    vainqueur = pi;
		    printf("I was elected %d\n", vainqueur);
		    faire_suivre(2, vainqueur, 0, -1, pj, neighbors, graph_comm);
		}
        }

    }






    //A la reception de Terminer(pk)
    if (message[0] == 2)
    {
        if (vainqueur != message[1])
        {
            faire_suivre(2, message[1], 0, -1, pj, neighbors, graph_comm);
            vainqueur = message[1];
            printf("Vainqueur est %d\n", vainqueur);
            etat = NON_CONCERNE;
            break;
        }
    }      

}        
    MPI_Finalize();
    return 0;
}
