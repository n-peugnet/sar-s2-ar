#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>

//************   LES TAGS
#define INIT_ELECTION 0 // Ask for election
#define ELECTED       1 // Inform others that

//************   LES VARIABLES MPI
int NB;                 // nb total de processus
int rank;               // mon identifiant
int left, right;        // les identifiants de mes voisins gauche et droit
int running = 1;
int initiator;

//************   LES FONCTIONS   ***************************

void receive_message(MPI_Status *status, int *value)
{
	MPI_Recv(value, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
}

void send_message(int dest, int tag, int val)
{
	// printf("P%d> Send message %d to %d\n", rank, tag, dest);
	MPI_Send(&val, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

int check_termination() {
	return running;
}

//************   LE PROGRAMME   ***************************
int main(int argc, char *argv[])
{

	MPI_Status status;
	int value;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NB);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	left = (rank + NB - 1) % NB;
	right = (rank + 1) % NB;
	printf("P%d> Started\n", rank);

	srand(getpid());
	initiator = rand() % 2;

	if (initiator) {
		printf("P%d> Is initiator\n", rank);
		send_message(right, INIT_ELECTION, rank);
	}

	while (check_termination()) {
		receive_message(&status, &value);
		if (status.MPI_TAG == INIT_ELECTION) {
			// Ask for election
			if (initiator && value < rank) {
				printf("P%d> Deleted the token of proc %d\n", rank, value);
			} else if (initiator && value == rank) {
				send_message(right, ELECTED, rank);
			} else {
				send_message(right, INIT_ELECTION, value);
			}
		} else if (status.MPI_TAG == ELECTED) {
			// Someone has been elected
			printf("P%d> Elected the proc %d\n", rank, value);
			send_message(right, ELECTED, value);
			running = 0;
		}
	}

	MPI_Finalize();
	return 0;
}
