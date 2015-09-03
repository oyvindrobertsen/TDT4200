.PHONY: clean all

START:=2
STOP:=100
PROCESSES:=16

serial: computeSerial.c
	gcc -g -o $@ computeSerial.c -std=c99 -lm -O2

run_serial: serial
	./serial $(START) $(STOP)

parallel: computeMPI.c
	mpicc -g -o $@ computeMPI.c -std=c99 -lm

run_parallel: parallel
	mpiexec -np $(PROCESSES) ./parallel $(START) $(STOP)

all: parallel

clean:
	rm -f serial parallel
