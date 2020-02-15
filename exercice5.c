#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "mpi_server.h"
#define MASTER 0
#define SIZE 128


void message_handler(int tag, int source) {
    int flag = 0;
    MPI_Status status;
    char message[SIZE];
    while(1) {
        pthread_mutex_lock(getMutex());
        MPI_Iprobe(source, tag, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            MPI_Recv(message, SIZE, MPI_CHAR, source, tag, MPI_COMM_WORLD, &status);
            printf("Message received %s\n", message);
        }
        pthread_mutex_unlock(getMutex());
    }
}

int main(int argc, char **argv){
    int my_rank;
    int nb_proc;
    int source;
    int dest;
    int tag =0;
    char message[SIZE];
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_SERIALIZED);

    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
    start_server(message_handler);
    
    sprintf(message, "Hello neighbour from %d", my_rank);
    dest = (my_rank + 1) % nb_proc;
    MPI_Send(message, strlen(message)+1, MPI_CHAR,dest,tag,MPI_COMM_WORLD);

    MPI_Recv(message, SIZE, MPI_CHAR, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
    printf("%s, on node %d\n", message, my_rank);

    MPI_Finalize();
    return 0;
}