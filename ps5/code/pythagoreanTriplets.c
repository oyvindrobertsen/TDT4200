#include <stdio.h> // for stdin
#include <stdlib.h>
#include <unistd.h> // for ssize_t

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#ifdef HAVE_OPENMP
#include <omp.h>
#endif
/*
int gcd(int a, int b) {
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
*/

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

    /*
     *	Remember to only print 1 (one) sum per start/stop.
     *	In other words, a total of <amountOfRuns> sums/printfs.
     */

    for (int i = 0; i < amountOfRuns; i++) {
        // Current run
        int total_ppt = 0;
        //printf("start: %d, stop: %d, threads: %d\n", start[i], stop[i], numThreads[i]);
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
