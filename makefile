all: dash.c
	gcc -g -Wall -o output dash.c
clean:
	output
