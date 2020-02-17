#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Nb requests */
#define MAX_CS 5

/* msg TYPES */
#define REQUEST 0
#define REPLY 1
#define END 2

/* states */
#define ENDED 0
#define WORKING 1

int count = 0;
int nb_reply = 0;
int nb_end = 0;
int clock = 0;
int clock_req = 0;

int rank;
int size;
int nb_request = 0;
int *requests;


void receive_message(MPI_Status *status, int *clock_msg)
{
	// printf("P%d> Waiting for message at %d\n", rank, clock);
	MPI_Recv(clock_msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
	// printf("P%d> Received %d at %d\n", rank, status->MPI_TAG, clock);
    clock = clock > *clock_msg ? clock + 1 : *clock_msg + 1;
}

void send_message(int dest, int tag)
{
    clock++;
	MPI_Send(&clock, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	printf("P%d> Sent %d at %d\n", rank, tag, clock);
}

void bcast_message(int tag)
{
    clock++;
    for (int i = 0; i < size; i++)
    {
        if (i == rank)
            continue;
	    MPI_Send(&clock, 1, MPI_INT, i, tag, MPI_COMM_WORLD);
    }
	printf("P%d> Broadcasted %d at %d\n", rank, tag, clock);
}

void wait_message() {
    MPI_Status status;
    int clock_msg;
    receive_message(&status, &clock_msg);
    switch(status.MPI_TAG) {
    case REPLY:
        nb_reply++;
        break;
    case REQUEST:
        if (clock_msg < clock || (clock_msg == clock && status.MPI_SOURCE < rank)) {
            send_message(status.MPI_SOURCE, REPLY);
        } else {
            nb_request++;
            requests[nb_request] = status.MPI_SOURCE;
        }
        break;
    case END:
        nb_end++;
        break;
    }
}

void request_cs()
{
	printf("P%d> Requesting at %d\n", rank, clock);
    bcast_message(REQUEST);
}

void release_cs()
{
	printf("P%d> Releasing %d at %d\n", rank, nb_request, clock);
    for (int i = 0; i < nb_request; i++)
    {
        send_message(requests[i], REPLY);
    }
    nb_reply = 0;
}

int main(int argc, char * argv[])
{
    MPI_Init(&argc, &argv);
    
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    requests = (int *) malloc(sizeof(int) * size);

    while (count < MAX_CS) {
        request_cs();
        while (nb_reply < size - 1) {
            wait_message();
        }
        /* Is in CS */
	    printf("P%d> In CS at %d\n", rank, clock);
        sleep(1);
        count++;
        release_cs();
    }
    bcast_message(END);
    while(nb_end < size - 1) {
        wait_message();
    }

	MPI_Finalize();
    return 0;
}
