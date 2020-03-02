EXECUTABLES := exercice2 TME2 TME3 #TME5 #exercice5

.PHONY: all
all: $(EXECUTABLES)
	
exercice2: exercice2.c
	mpicc $^ -o $@

exercice5: exercice5.c mpi_server.c
	mpicc $^ -o $@

TME2: TME2.c
	mpicc $^ -o $@

TME3: TME3.c
	mpicc $^ -o $@

TME5: TME5.c
	mpicc $^ -o $@

.PHONY: clean
clean:
	rm -rf $(EXECUTABLES)
