#include <stdio.h> // for stdin
#include <stdlib.h>
#include <unistd.h> // for ssize_t

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#ifdef HAVE_OPENMP
#include <omp.h>
#endif

int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main(int argc, char **argv) {
    char *inputLine = NULL; size_t lineLength = 0;
    int *start, *stop, *numThreads, amountOfRuns = 0;

    // Read in first line of input
    getline(&inputLine, &lineLength, stdin);
    sscanf(inputLine, "%d", &amountOfRuns);

    stop = (int*) calloc(amountOfRuns, sizeof(int));
    start = (int*) calloc(amountOfRuns, sizeof(int));
    numThreads = (int*) calloc(amountOfRuns, sizeof(int));

    int tot_threads, current_start, current_stop;
    for (int i = 0; i < amountOfRuns; ++i){

        // Read in each line of input that follows after first line
        free(inputLine); lineLength = 0; inputLine = NULL;
        ssize_t readChars = getline(&inputLine, &lineLength, stdin);

        // If there exists at least two matches (2x %d)...
        if (sscanf(inputLine, "%d %d %d", &current_start, &current_stop, &tot_threads) >= 2){
            if(current_start < 0 || current_stop < 0){
                current_start = 0, current_stop = 0;
            }
            stop[i] = current_stop;
            start[i] = current_start;
            numThreads[i] = tot_threads;
        }
    }

    /*
     *	Remember to only print 1 (one) sum per start/stop.
     *	In other words, a total of <amountOfRuns> sums/printfs.
     */

    for (int i = 0; i < amountOfRuns; i++) {
        // Current run
        int total_ppt = 0;
        #pragma omp parallel for num_threads(numThreads[i]) \
            reduction(+: total_ppt)
        for (int c = start[i]; c < stop[i]; c++) {
            for (int b = 4; b < c; b++) {
                for (int a = 3; a < b; a++) {
                    if ((a*a + b*b == c*c) && gcd(a, b) == 1 && gcd(b, c) == 1) {
                        total_ppt++;
                    }
                }
            }
        }
        printf("%d\n", total_ppt);
    }
    return 0;
}
