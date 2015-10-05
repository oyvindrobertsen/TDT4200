.PHONY: clean

CC:=clang
libs:=-lOpenCL -lm
CFLAGS:=--std=c99 -O2

image.png: program
	./$< < input_tdsotm.txt > $@

program: pinkfloyd.o lodepng.o
	$(CC) $^ -o $@ $(libs)

mac_program: pinkfloyd.o lodepng.o
	$(CC) $^ -o $@ -lm -framework OpenCL
clean:
	rm -f *.o program mac_program image.png
