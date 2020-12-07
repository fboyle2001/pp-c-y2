all: add.o subtract.o libmaths.so maths_prog.o
	gcc -Wextra -Wall -pedantic -std=c11 maths_prog.o -L. -lmaths -o maths_prog

maths_prog.o: maths_prog.c
	gcc -Wextra -Wall -pedantic -std=c11 -c maths_prog.c

libmaths.so: add.o subtract.o
	gcc -Wextra -Wall -pedantic -std=c11 -shared add.o subtract.o -o libmaths.so

add.o: add.c add.h
	gcc -Wextra -Wall -pedantic -std=c11 -fPIC -c add.c

subtract.o: subtract.c subtract.h
	gcc -Wextra -Wall -pedantic -std=c11 -fPIC -c subtract.c

clean:
	rm -rf maths_prog add.o subtract.o libmaths.so maths_prog.o
