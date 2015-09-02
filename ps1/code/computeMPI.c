#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/*
    A simple MPI example.

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

    // Start timing
    double local_start_time, local_finish_time, local_elapsed_time, elapsed_time;

    MPI_Barrier(MPI_COMM_WORLD);
    local_start_time = MPI_Wtime();

    int partition_size = (int) ((stop - start) / size);
    int local_start = start + (rank * partition_size);
    int local_stop = start + ((rank + 1) * partition_size);
    if (rank == size - 1) {
        local_stop += stop - local_stop;
    }

    //printf("Process %d, partition_size = %d, local_start = %d, local_stop = %d\n", rank, partition_size, local_start, local_stop);

    // Perform the computation
    double local_sum = 0.0;
    for (int i = local_start; i < local_stop; i++) {
        local_sum += 1.0/log(i);
    }

    if (rank != 0) {
        MPI_Send(&local_sum, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        local_finish_time = MPI_Wtime();
        local_elapsed_time = local_finish_time - local_start_time;
        MPI_Reduce(&local_elapsed_time, &elapsed_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    } else {
        double rec_val;
        for (int src = 1; src < size; src++) {
            MPI_Recv(&rec_val, 1, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_sum += rec_val;
        }
        local_finish_time = MPI_Wtime();
        local_elapsed_time = local_finish_time - local_start_time;
        MPI_Reduce(&local_elapsed_time, &elapsed_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
        printf("%f\n", local_sum);
        printf("Elapsed time: %f seconds\n", elapsed_time);
    }

    MPI_Finalize();
    return 0;
}

