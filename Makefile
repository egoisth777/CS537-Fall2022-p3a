# Makefile

default:
	gcc -g psort.c -o psort -Wall -Werror -pthread -O

gdb:default
	gdb --args psort ./inputfiles/input1.bin
test:
	gcc -g test.c -o test 

tests:
	gcc -g testReadin.c test.c -o testReadin