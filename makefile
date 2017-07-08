all: dash.c
	cc -g -Wall -o output dash.c
clean:
	output
