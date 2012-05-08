EXE_SEQ=../bin/sort.seq
EXE_MPI=../bin/sort.mpi
RAD_SEQ=../bin/radix.seq
RAD_MPI=../bin/radix.mpi
RAD_BAL=../bin/radix.bal.mpi
EXE_TST=../bin/tests

.PHONY: $(EXE_SEQ) $(EXE_OMP) $(EXE_MPI) $(EXE_TST)

CC=g++
LIB=-lrt
FLAGS=-O3

default: all

all: radix radix.mpi radix.balanced.mpi

tests: test.cpp
	g++ -Wall test.cpp common.cpp -o $(EXE_TST) $(LIB)

radix: common.o radix.seq.o
	g++ -Wall common.o radix.seq.o -o $(RAD_SEQ) $(LIB)

radix.mpi:
	mpic++ -Wall $(FLAGS) radix.mpi.cpp -c
	mpic++ -Wall $(FLAGS) common.o radix.mpi.o -o $(RAD_MPI) $(LIB)

radix.balanced.mpi:
	mpic++ -Wall $(FLAGS) radix.balanced.mpi.cpp -c
	mpic++ -Wall $(FLAGS) common.o radix.balanced.mpi.o -o $(RAD_BAL) $(LIB)

clean:
	$(RM) *.o
