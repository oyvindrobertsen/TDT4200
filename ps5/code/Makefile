.PHONY: serial mpi run_mpi omp mac_omp hybrid clean

all: serial omp

serial:
	gcc -g -std=c99 -o serial pythagoreanTriplets.c

omp:
	gcc -fopenmp -DHAVE_OPENMP -std=c99 -o omp pythagoreanTriplets.c

mac_omp:
	gcc-4.8 -fopenmp -DHAVE_OPENMP -std=c99 -o omp pythagoreanTriplets.c

mpi:
	mpicc -g -DHAVE_MPI -o $@ pythagoreanTriplets.c

run_mpi: mpi
	mpiexec -np 4 ./mpi < input.txt

hybrid:
	mpicc -g -fopenmp -DHAVE_MPI -DHAVE_OPENMP -o $@ pythagoreanTriplets.c

clean:
	rm -rf serial omp mpi *.dSYM hybrid
