# Makefile

default:
	gcc -g psort.c -o psort
gdb:default
	
test:
	gcc -g test.c -o test

tests:
	gcc -g testReadin.c test.c -o testReadin