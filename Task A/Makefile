all: add.o subtract.o maths_prog.c
	gcc -Wextra -Wall -pedantic -std=c11 -o maths_prog maths_prog.c add.o subtract.o

add.o: add.c add.h
	gcc -Wextra -Wall -pedantic -std=c11 -c add.c

subtract.o: subtract.c subtract.h
	gcc -Wextra -Wall -pedantic -std=c11 -c subtract.c

clean:
	rm -rf maths_prog add.o subtract.o
