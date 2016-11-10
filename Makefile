SRCS = main.c
OBJS = $(SRCS:.cpp=.o)

build:
	mpicxx main.c -o hello -lm -std=c++11
	
run: build
	mpirun -np 4 ./hello 1 1000000 out.txt

all: run
	
report: build
	echo "Primes: " > out.txt
	echo "" > ./reportdata
	for i in 1 2 3 4 5 6 7 8 9 10; do \
		mpirun -np $$i ./hello 1 700000 out.txt >> ./reportdata; \
	done;
	gnuplot plotrules

	
clean: 
	rm -rf *.o hello