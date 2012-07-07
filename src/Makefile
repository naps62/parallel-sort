EXE_SEQ=../bin/sort.seq
EXE_MPI=../bin/sort.mpi

RAD_SEQ=../bin/radix.seq
RAD_OMP=../bin/radix.omp
RAD_MPI=../bin/radix.mpi
RAD_BAL=../bin/radix.bal.mpi
RAD_TBB=../bin/radix.tbb
EXE_TST=../bin/tests

.PHONY: $(EXE_SEQ) $(EXE_OMP) $(EXE_MPI) $(EXE_TST)

CC=g++
LIB=-lrt
CFLAGS= -Wall -O3

default: radix.omp radix.tbb

all: radix radix.mpi radix.omp radix.balanced.mpi gen

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

gen:
	g++ $(CFLAGS) gen.cpp -o ../bin/gen

tests: test.cpp
	g++ $(CFLAGS) test.cpp common.cpp -o $(EXE_TST) $(LIB)

radix: common.o radix.seq.o
	g++ common.o radix.seq.o -o $(RAD_SEQ) $(LIB)

radix.omp: common.o
	g++ $(CFLAGS) -c radix.omp.cpp -fopenmp
	g++ $(CFLAGS) common.o radix.omp.o -o $(RAD_OMP) $(LIB) -fopenmp

radix.tbb: common.o
	g++ $(CFLAGS) -c radix.tbb.cpp -ltbb
	g++ $(CFLAGS) common.o radix.tbb.o $(RAD_TBB) $(LIB) -ltbb

radix.mpi:
	mpic++ $(CFLAGS) radix.mpi.cpp -c
	mpic++ $(CFLAGS) common.o radix.mpi.o -o $(RAD_MPI) $(LIB)

radix.balanced.mpi:
	mpic++ $(CFLAGS) radix.balanced.mpi.cpp -c
	mpic++ $(CFLAGS) common.o radix.balanced.mpi.o -o $(RAD_BAL) $(LIB)

clean:
	$(RM) *.o
