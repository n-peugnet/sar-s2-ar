EXECUTABLES := exercice2 TME2_philo #exercice5

.PHONY: all
all: $(EXECUTABLES)
	
exercice2: exercice2.c
	mpicc $^ -o $@

exercice5: exercice5.c mpi_server.c
	mpicc $^ -o $@

TME2_philo: TME2_philo.c
	mpicc $^ -o $@

TME4: TME4.c
	mpicc $^ -o $@

.PHONY: clean
clean:
	rm -rf $(EXECUTABLES)
