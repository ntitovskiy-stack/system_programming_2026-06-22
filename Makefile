all: solution

solution: solution.c
	gcc -o solution solution.c
clean:
	rm -f solution
