.PHONY: clean all

START:=3
STOP:=10000000
PROCESSES:=16

serial: src/computeSerial.c
	gcc -o bin/$@ src/computeSerial.c -std=c99 -lm -O2

run_serial: serial
	./bin/serial $(START) $(STOP)

parallell: src/computeMPI.c
	mpicc -o bin/$@ src/computeMPI.c -std=c99 -lm

run_parallell: parallell
	mpiexec -n $(PROCESSES) ./bin/parallell $(START) $(STOP)

clean:
	rm -f bin/serial bin/parallell
