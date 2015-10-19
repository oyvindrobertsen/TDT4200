.PHONY: serial mpi omp hybrid clean

serial:
	gcc -g -Wall -o serial pythagoreanTriplets.c

omp:
	gcc-4.8 -fopenmp -DHAVE_OPENMP -std=c99 -o omp pythagoreanTriplets.c

clean:
	rm -f serial serial.dSYM omp
