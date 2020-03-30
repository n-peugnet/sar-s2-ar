#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define TAG_INIT 0
#define TAG_WAKE 1
#define TAG_ELEC 2

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
        MPI_Send(&nb_voisins[i], 1, MPI_INT, i, TAG_INIT, MPI_COMM_WORLD);
        MPI_Send(voisins[i], nb_voisins[i], MPI_INT, i, TAG_INIT, MPI_COMM_WORLD);
        // MPI_Send(&min_local[i], 1, MPI_INT, i, TAG_INIT, MPI_COMM_WORLD);
    }
}

void send_message(int dest, int tag, int min)
{
    MPI_Send(&min, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

/* main d'un processus */
void calcul_min(int rank)
{
    ////////////////////////////////////// Init /////////////////////////////////////////////
    int sent = 0, min_local, last, nb_voisins, min_recv, i;
    int recv[NB_SITE + 1];
    int *voisins;
    int initiator = 0, waked = 0, nb_recv = 0;

    srand(time(NULL));
    min_local = rank;
    MPI_Status status;
    MPI_Recv(&nb_voisins, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INIT, MPI_COMM_WORLD, &status);
    voisins = (int *) malloc(sizeof(int) * nb_voisins);
    MPI_Recv(voisins, nb_voisins, MPI_INT, MPI_ANY_SOURCE, TAG_INIT, MPI_COMM_WORLD, &status);
    // MPI_Recv(&min_local, 1, MPI_INT, MPI_ANY_SOURCE, TAG_INIT, MPI_COMM_WORLD, &status);
    printf("P%d> nbvoisins: %d, voisin0: %d, min_local: %d\n", rank, nb_voisins, voisins[0], min_local);

    srand(getpid());
    initiator = rand() % 2;

    if (initiator) {
        waked = 1;
        printf("P%d> Is initiator\n", rank);
        for (i = 0; i < nb_voisins; i++) {
            send_message(voisins[i], TAG_WAKE, 0);
            printf("P%d> sent WAKEUP! to %d\n", rank, voisins[i]);
        }
        if (nb_voisins == 1) {
            sent++;
            send_message(voisins[0], TAG_ELEC, min_local);
            printf("P%d> sent %d to %d\n", rank, min_local, voisins[0]);
        }
    }

    while (nb_recv < nb_voisins)
    {
        ////////////////////////////////////// R(p) /////////////////////////////////////////////
        MPI_Recv(&min_recv, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == TAG_WAKE) {
            printf("P%d> received WAKEUP! from %d\n", rank, status.MPI_SOURCE);
            if (!waked) {
                waked = 1;
                // Si on est une feuille.
                if (nb_voisins == 1) {
                    sent++;
                    send_message(voisins[0], TAG_ELEC, min_local);
                    printf("P%d> sent %d to %d\n", rank, min_local, voisins[0]);
                } else {
                    for (i = 0; i < nb_voisins; i++) {
                        if (voisins[i] == status.MPI_SOURCE)
                            continue;
                        send_message(voisins[i], TAG_WAKE, 0);
                        printf("P%d> sent WAKEUP! to %d\n", rank, voisins[i]);
                    }
                }
            }
        } else if (status.MPI_TAG == TAG_ELEC) {
            printf("P%d> received %d from %d\n", rank, min_recv, status.MPI_SOURCE);
            if (min_recv < min_local) {
                min_local = min_recv;
            }
            nb_recv++;
            recv[status.MPI_SOURCE] = 1;
            last = status.MPI_SOURCE;
        }

        ////////////////////////////////////// D(p) /////////////////////////////////////////////
        if (nb_recv == nb_voisins) {
            for (i = 0; i < nb_voisins; i++) {
                if (voisins[i] == last)
                    continue;
                sent++;
                send_message(voisins[i], TAG_ELEC, min_local);
                printf("P%d> sent %d to %d\n", rank, min_local, voisins[i]);
            }
        }

        ////////////////////////////////////// S(p) /////////////////////////////////////////////
        if (sent == 0 && nb_recv == nb_voisins - 1) {
            for (i = 0; i < nb_voisins; i++) {
                if (recv[voisins[i]] == 1)
                    continue;
                sent++;
                send_message(voisins[i], TAG_ELEC, min_local);
                printf("P%d> sent %d to %d\n", rank, min_local, voisins[i]);
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