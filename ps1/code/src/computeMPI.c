#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/*
	A simple MPI example.
	TODO:
	1. Fill in the needed MPI code to make this run on any number of nodes.
	2. The answer must match the original serial version.
	3. Think of corner cases (valid but tricky values).

	Example input:
	./simple 2 10000

*/

int main(int argc, char **argv) {
	int rank, size;

	if (argc < 3) {
		printf("This program requires two parameters:\n \
the start and end specifying a range of positive integers in which \
start is 2 or greater, and end is greater than start.\n");
		exit(1);
	}

	int start = atoi(argv[1]);
	int stop = atoi(argv[2]);


	if(start < 2 || stop <= start){
		printf("Start must be greater than 2 and the end must be larger than start.\n");
		exit(1);
	}

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int partition_size = ((int) stop - start) / size;
    int local_start = start + (rank * partition_size);
    int local_stop = start + ((rank + 1) * partition_size);
    if (rank == size - 1) {
        local_stop += stop - local_stop;
    }

    //printf("Process %d, partition_size = %d, local_start = %d, local_stop = %d\n", rank, partition_size, local_start, local_stop);
	// TODO: Compute the local range, so that all the elements are accounted for.


	// Perform the computation
	double local_sum = 0.0;
	for (int i = local_start; i < local_stop; i++) {
		local_sum += 1.0/log(i);
	}


    if (rank != 0) {
        MPI_Send(&local_sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    } else {
        double rec_val;
        for (int src = 1; src < size; src++) {
            MPI_Recv(&rec_val, 1, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_sum += rec_val;
        }
	    printf("The sum is: %f\n", local_sum);
    }

    MPI_Finalize();
	return 0;
}

