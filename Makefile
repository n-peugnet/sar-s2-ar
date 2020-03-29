EXECUTABLES := TME1E2 TME2 TME3 TME5 TME6E1 TME6E2 #TME1E5

.PHONY: all
all: $(EXECUTABLES)

TME1E5: TME1E5.c mpi_server.c
	mpicc $^ -o $@

TME%: TME%.c
	mpicc $^ -o $@

.PHONY: clean
clean:
	rm -rf $(EXECUTABLES)
