.PHONY: serial mpi omp hybrid clean

all: serial omp

serial:
	gcc -g -std=c99 -o serial pythagoreanTriplets.c

omp:
	gcc -fopenmp -DHAVE_OPENMP -std=c99 -o omp pythagoreanTriplets.c

clean:
	rm -rf serial serial.dSYM omp
