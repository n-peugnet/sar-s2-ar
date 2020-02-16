#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

//************   LES TAGS
#define WANNA_CHOPSTICK 0		// Demande de baguette
#define CHOPSTICK_YOURS 1		// Cession de baguette
#define DONE_EATING     2		// Annonce de terminaison

//************   LES ETATS D'UN NOEUD
#define THINKING 0   // N'a pas consomme tous ses repas & n'attend pas de baguette
#define HUNGRY   1   // En attente de baguette
#define DONE     2   // A consomme tous ses repas

//************   LES REPAS
#define NB_MEALS 3	// nb total de repas a consommer par noeud

//************   MACROS BONUS 
#define DURATION 30

//************   LES VARIABLES MPI
int NB;               // nb total de processus
int rank;             // mon identifiant
int left, right;      // les identifiants de mes voisins gauche et droit

//************   LA TEMPORISATION
int local_clock = 0;                    // horloge locale
int clock_val;                          // valeur d'horloge recue
int meal_times[NB_MEALS];        // dates de mes repas

//************   LES ETATS LOCAUX
int local_state = HUNGRY;		// moi
int left_state  = HUNGRY;		// mon voisin de gauche
int right_state = HUNGRY;		// mon voisin de droite

//************   LES BAGUETTES 
int left_chopstick = 0;		// je n'ai pas celle de gauche
int right_chopstick = 0;	// je n'ai pas celle de droite

//************   LES REPAS 
int meals_eaten = 0;		// nb de repas consommes localement

//************   LES FONCTIONS   ***************************
int max(int a, int b)
{
	return (a > b ? a : b);
}

void receive_message(MPI_Status *status)
{
	MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, status);
}

void send_message(int dest, int tag)
{
	printf("P%d> Send message %d to %d\n", rank, tag, dest);
	MPI_Send(NULL, 0, MPI_INT, dest, tag, MPI_COMM_WORLD);
}

/* renvoie 0 si le noeud local et ses 2 voisins sont termines */
int check_termination()
{
	if (left_state == DONE && right_state == DONE && meals_eaten == NB_MEALS) {
		return 0;
	}
	return 1;
}

void eat()
{
	printf("P%d> Eating\n", rank);
	sleep(1);
	if (++meals_eaten == NB_MEALS) {
		local_state = DONE;
		send_message(left, DONE_EATING);
		send_message(right, DONE_EATING);
	}
	left_chopstick = 0; // je n'ai pas celle de gauche
	right_chopstick = 0; // je n'ai pas celle de droite
}

void change_state()
{
	if (local_state == HUNGRY) {
		printf("P%d> Thinking\n", rank);
		local_state = THINKING;
		left_chopstick = 0;
		right_chopstick = 0;
	} else {
		printf("P%d> Waiting\n", rank);
		local_state = HUNGRY;
	}
}

//************   LE PROGRAMME   ***************************
int main(int argc, char *argv[])
{

	MPI_Status status;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &NB);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	left = (rank + NB - 1) % NB;
	right = (rank + 1) % NB;

	while (check_termination()) {

		/* Tant qu'on n'a pas fini tous ses repas, redemander les 2 baguette
		   a chaque fin de repas */
		if ((meals_eaten < NB_MEALS) && (local_state == HUNGRY)) {
			//demande de baguette aux 2 voisins
			send_message(right, WANNA_CHOPSTICK);
			send_message(left, WANNA_CHOPSTICK);
			printf("P%d> Asking for chopsticks at %d\n", rank, local_clock);
		}

		receive_message(&status);

		if (status.MPI_TAG == DONE_EATING) {
			//Enregistrer qu'un voisin a fini de manger
			if (left == status.MPI_SOURCE) {
				left_state = DONE;
			} else if (right == status.MPI_SOURCE) {
				right_state = DONE;
			}
		} else if (status.MPI_TAG == CHOPSTICK_YOURS) {
			if (local_state == HUNGRY) {
				if (left == status.MPI_SOURCE) {
					left_chopstick = 1;
				} else if (right == status.MPI_SOURCE) {
					right_chopstick = 1;
				}
				if (left_chopstick == 1 && right_chopstick == 1) {
					eat();
				}
			}
		} else {
			/* c'est une demande */
			if (local_state != HUNGRY) {
				send_message(status.MPI_SOURCE, CHOPSTICK_YOURS);
			} else if (rank > 0 && rank > status.MPI_SOURCE) {
				send_message(status.MPI_SOURCE, CHOPSTICK_YOURS);
				if (left == status.MPI_SOURCE)
					left_chopstick = 0;
				else if (right == status.MPI_SOURCE)
					right_chopstick = 0;
			}
		}

		// Change between HUNGRY and THINKING
		local_clock++;
		if (local_clock % DURATION == 0)
			change_state();
	}

	MPI_Finalize();
	return 0;
}
