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
OMP=-fopenmp
TBB=-ltbb

default: radix radix.omp radix.tbb

all: radix radix.mpi radix.omp radix.balanced.mpi gen

gen:
	g++ $(CFLAGS) gen.cpp -o ../bin/gen

tests: test.cpp
	g++ $(CFLAGS) test.cpp common.cpp -o $(EXE_TST) $(LIB)

radix: common.o radix.seq.o
	$(CC) common.o radix.seq.o -o $(RAD_SEQ) $(LIB)

radix.omp: common.o
	$(CC) $(CFLAGS) -c radix.omp.cpp $(OMP)
	$(CC) $(CFLAGS) common.o radix.omp.o -o $(RAD_OMP) $(LIB) $(OMP)

radix.tbb: common.o
	$(CC) $(CFLAGS) -c radix.tbb.cpp
	$(CC) $(CFLAGS) common.o radix.tbb.o -o $(RAD_TBB) $(LIB) $(TBB)

radix.mpi:
	mpic++ $(CFLAGS) radix.mpi.cpp -c
	mpic++ $(CFLAGS) common.o radix.mpi.o -o $(RAD_MPI) $(LIB)

radix.balanced.mpi:
	mpic++ $(CFLAGS) radix.balanced.mpi.cpp -c
	mpic++ $(CFLAGS) common.o radix.balanced.mpi.o -o $(RAD_BAL) $(LIB)

clean:
	$(RM) *.o


.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

