#define _GNU_SOURCE
#include <stdio.h> // for stdin
#include <stdlib.h>
#include <unistd.h> // for ssize_t

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#ifdef HAVE_OPENMP
#include <omp.h>
#endif

// GCD-implementation taken from
// https://en.wikipedia.org/wiki/Binary_GCD_algorithm#Iterative_version_in_C
unsigned int gcd(unsigned int u, unsigned int v)
{
    int shift;

    /* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
    if (u == 0) return v;
    if (v == 0) return u;

    /* Let shift := lg K, where K is the greatest power of 2
     *         dividing both u and v. */
    for (shift = 0; ((u | v) & 1) == 0; ++shift) {
        u >>= 1;
        v >>= 1;

    }

    while ((u & 1) == 0)
        u >>= 1;

    /* From here on, u is always odd. */
    do {
        /* remove all factors of 2 in v -- they are not common */
        /*   note: v is not zero, so while will terminate */
        while ((v & 1) == 0)  /* Loop X */
            v >>= 1;

        /* Now u and v are both odd. Swap if necessary so u <= v,
         *           then set v = v - u (which is even). For bignums, the
         *                     swapping is just pointer movement, and the subtraction
         *                               can be done in-place. */
        if (u > v) {
            unsigned int t = v; v = u; u = t;
        }  // Swap u and v.
        v = v - u;                       // Here v >= u.

    } while (v != 0);

    /* restore common factors of 2 */
    return u << shift;

}


int main(int argc, char **argv) {
    // Setup common data structures, parse input
    char *inputLine = NULL; size_t lineLength = 0;
    int *start, *stop, *numThreads, amountOfRuns = 0;

    stop = (int*) calloc(amountOfRuns, sizeof(int));
    start = (int*) calloc(amountOfRuns, sizeof(int));
    numThreads = (int*) calloc(amountOfRuns, sizeof(int));


    // MPI setup
    int rank = 0;
#ifdef HAVE_MPI
    int size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

#endif

    if (rank == 0) {
        // parse and read input in master process
        // Read in first line of input
        getline(&inputLine, &lineLength, stdin);
        sscanf(inputLine, "%d", &amountOfRuns);

        int tot_threads, current_start, current_stop;
        for (int i = 0; i < amountOfRuns; ++i){

            // Read in each line of input that follows after first line
            free(inputLine); lineLength = 0; inputLine = NULL;
            ssize_t readChars = getline(&inputLine, &lineLength, stdin);

            int matches = sscanf(inputLine, "%d %d %d", &current_start, &current_stop, &tot_threads);
            // If there exists at least two matches (2x %d)...
            if (matches == 2) {
                tot_threads = 1;
            }
            if (matches >= 2){
                if(current_start < 0 || current_stop < 0){
                    current_start = 0, current_stop = 0;
                }
                stop[i] = current_stop;
                start[i] = current_start;
                numThreads[i] = tot_threads;
            }
        }
    }

#ifdef HAVE_MPI
    // Broadcast information parsed from input to other processes
    MPI_Bcast(&amountOfRuns, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(start, amountOfRuns, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(stop, amountOfRuns, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(numThreads, amountOfRuns, MPI_INT, 0, MPI_COMM_WORLD);
#endif

    for (int i = 0; i < amountOfRuns; i++) {
        // Current run
        int local_ppt = 0;
        int current_start = start[i];
        int current_stop = stop[i];
        int local_start = 2;
        int local_stop = current_stop;
        if (current_start > current_stop) {
            if (rank == 0) {
                printf("0\n");
                continue;
            }
        }

#ifdef HAVE_MPI
        // Calculate local range
        int local_range_size = (int) ((current_stop) / size);
        local_start = 2 + (rank * local_range_size);
        local_stop = 2 + (rank+1 * local_range_size);
        if (rank == size - 1) {
            local_stop += current_stop - local_stop;
        }
#endif

#pragma omp parallel for num_threads(numThreads[i]) reduction(+: local_ppt) schedule(guided)
        for (int m = local_start; m < local_stop; m++) {
            for (int n = 1; n < m; n++) {
                int c = m*m + n*n;                
                if (c >= current_stop) {
                    // n too large
                    break;
                }
                if (c < current_start) {
                    // n too small
                    continue;
                }
                if (gcd(m, n) == 1 && (m - n) & 1) {
                    // m, n are coprime and both are not odd
                    // -> valid primitive pythagorean triplet
                    //printf("%d, %d, %d, %d\n", i, m*m - n*n, 2*m*n, c);
                    local_ppt++;
                }
            }
        }
#ifdef HAVE_MPI
        // Gather results and print total
        int total_ppt = 0;
        MPI_Reduce(&local_ppt, &total_ppt, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("%d\n", total_ppt);
        }
#else
        printf("%d\n", local_ppt);
#endif
    }

#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 0;
}
