.PHONY: clean all

START:=2
STOP:=10000000
PROCESSES:=2

serial: computeSerial.c
	gcc -o $@ computeSerial.c -std=c99 -lm -O2

run_serial: serial
	./serial $(START) $(STOP)

parallel: computeMPI.c
	mpicc -o $@ computeMPI.c -std=c99 -lm

run_parallel: parallel
	mpiexec -np $(PROCESSES) ./parallel $(START) $(STOP)

all: parallel

clean:
	rm -f serial parallel
