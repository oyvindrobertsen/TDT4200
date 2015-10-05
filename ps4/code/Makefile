.PHONY: clean

CC:=clang
libs:=-lOpenCL -lm
mac_libs:=-lm -framework OpenCL
CFLAGS:=--std=c99 -g -O2

image.png: program
	./$< < input_tdsotm.txt > $@

program: pinkfloyd.o lodepng.o
	$(CC) $^ -o $@ $(libs)

mac_program: pinkfloyd.o lodepng.o
	$(CC) $^ -o $@ $(mac_libs)

clean:
	rm -f *.o program mac_program image.png
