#include <stdio.h>
#include <string.h>
#include <mpi.h>
#define MASTER 0
#define SIZE 128
int main(int argc, char **argv){
    int my_rank;
    int nb_proc;
    int source;
    int dest;
    int tag =0;
    char message[SIZE];
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);

    if (my_rank == MASTER) {

        MPI_Recv(message, SIZE, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
        printf("%s, on node %d\n", message, my_rank);

        sprintf(message, "Hello neighbour from %d", my_rank);
        dest = (my_rank + 1) % nb_proc;
        MPI_Ssend(message, strlen(message)+1, MPI_CHAR,dest,tag,MPI_COMM_WORLD);

    } else {

        sprintf(message, "Hello neighbour from %d", my_rank);
        dest = (my_rank + 1) % nb_proc;
        MPI_Ssend(message, strlen(message)+1, MPI_CHAR,dest,tag,MPI_COMM_WORLD);

        MPI_Recv(message, SIZE, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
        printf("%s, on node %d\n", message, my_rank);

    }

    MPI_Finalize();
    return 0;
}