#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAGINIT 0
#define TAGMSG 1

#define NB_SITE 6


void simulateur(void)
{
    int i;

    /* nb_voisins[i] est le nombre de voisins du site i */
    int nb_voisins[NB_SITE + 1] = { -1, 2, 3, 2, 1, 1, 1 };
    int min_local[NB_SITE + 1] = { -1, 3, 11, 8, 14, 5, 17 };

    /* liste des voisins */
    int voisins[NB_SITE + 1][3] = { { -1, -1, -1 },
        { 2, 3, -1 }, { 1, 4, 5 },
        { 1, 6, -1 }, { 2, -1, -1 },
        { 2, -1, -1 }, { 3, -1, -1 } };

    for (i = 1; i <= NB_SITE; i++) {
        MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
        MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
        MPI_Send(&min_local[i], 1, MPI_INT, i, TAGINIT, MPI_COMM_WORLD);
    }
}

void send_message(int dest, int min)
{
    MPI_Send(&min, 1, MPI_INT, dest, TAGMSG, MPI_COMM_WORLD);
}

/* main d'un processus */
void calcul_min(int rank)
{
    ////////////////////////////////////// Init /////////////////////////////////////////////
    int sent, min_local, last, nb_voisins, nb_recv, min_recv, i;
    int recv[NB_SITE + 1];
    int *voisins;
    srand(time(NULL));
    min_local = rand() % 100;
    MPI_Status status;
    MPI_Recv(&nb_voisins, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    voisins = (int *) malloc(sizeof(int) * nb_voisins);
    MPI_Recv(voisins, nb_voisins, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    MPI_Recv(&min_local, 1, MPI_INT, MPI_ANY_SOURCE, TAGINIT, MPI_COMM_WORLD, &status);
    printf("P%d> nbvoisins: %d, voisin0: %d, min_local: %d\n", rank, nb_voisins, voisins[0], min_local);

    // Si on est une feuille.
    if (nb_voisins == 1) {
        send_message(voisins[0], min_local);
        printf("P%d> sent %d to %d\n", rank, min_local, voisins[0]);
    }

    while (nb_recv < nb_voisins)
    {
        ////////////////////////////////////// R(p) /////////////////////////////////////////////
        MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, TAGMSG, MPI_COMM_WORLD, &status);
        printf("P%d> received %d from %d\n", rank, min_recv, status.MPI_SOURCE);
        if (min_recv < min_local) {
            min_local = min_recv;
        }
        nb_recv++;
        recv[status.MPI_SOURCE] = 1;
        last = status.MPI_SOURCE;

        ////////////////////////////////////// D(p) /////////////////////////////////////////////
        if (nb_recv == nb_voisins) {
            for (i = 0; i < nb_voisins; i++) {
                if (voisins[i] == last)
                    continue;
                send_message(voisins[i], min_local);
                printf("P%d> sent %d to %d\n", rank, min_local, voisins[i]);
            }
        }

        ////////////////////////////////////// S(p) /////////////////////////////////////////////
        if (sent == 0 && nb_recv == nb_voisins - 1) {
            for (i = 0; i < nb_voisins; i++) {
                if (recv[voisins[i]] == 1)
                    continue;
                send_message(voisins[i], min_local);
            }
        }
    }



    free(voisins);
    printf("P%d> final min: %d\n", rank, min_local);
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    int nb_proc, rang;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);

    if (nb_proc != NB_SITE + 1) {
        printf("Nombre de processus incorrect !\n");
        MPI_Finalize();
        exit(2);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &rang);

    if (rang == 0) {
        simulateur();
    } else {
        calcul_min(rang);
    }

    MPI_Finalize();
    return 0;
}