# Linked List Sorting
all: llSort test

llSort: llSort.c Makefile
	gcc -O2 -s -o llSort llSort.c -Wall -Wno-unused-function

test:
	size llSort
	time ./llSort

clean:
	rm -f llSort
