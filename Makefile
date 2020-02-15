all: exercice2 exercice5 TME2_philo.c

exercice2: exercice2.c
	mpicc $^ -o $@

exercice5: exercice5.c mpi_server.c
	mpicc $^ -o $@

TME2_philo: TME2_philo.c
	mpicc $^ -o $@
