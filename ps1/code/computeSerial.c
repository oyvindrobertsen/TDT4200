#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

/*
	A simple SERIAL example.

    Example input:
    ./simple 2 10000

*/

int main(int argc, char **argv) {

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

    clock_t begin, end;
    double time_spent;
    begin = clock();

	// Perform the computation
	double sum = 0.0;
	for (int i = start; i < stop ; i++) {
		sum += 1.0/log(i);
	}

	// Debug prints if needed

	//Print the global sum once only
	printf("The sum is: %f\n", sum);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Elapsed time: %f seconds\n", time_spent);
	return 0;
}

