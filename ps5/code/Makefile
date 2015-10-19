.PHONY: serial mpi omp hybrid clean

serial:
	gcc -g -Wall -o serial pythagoreanTriplets.c

omp:
	gcc -fopenmp -DHAVE_OPENMP -std=c99 -o omp pythagoreanTriplets.c

clean:
	rm -rf serial serial.dSYM omp
