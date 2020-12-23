all: connect4.o main.o
	gcc -Wextra -Wall -pedantic -std=c11 connect4.o main.o -o main

main.o: main.c
	gcc -Wextra -Wall -pedantic -std=c11 -c -g main.c

connect4.o: connect4.c connect4.h
	gcc -Wextra -Wall -pedantic -std=c11 -c -g connect4.c

clean:
	rm -rf connect4.o main.o main
