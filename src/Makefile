EXE_SEQ=../bin/sort.seq
EXE_MPI=../bin/sort.mpi
RAD_SEQ=../bin/radix.seq
RAD_MPI=../bin/radix.mpi
EXE_TST=../bin/tests

.PHONY: $(EXE_SEQ) $(EXE_OMP) $(EXE_MPI) $(EXE_TST)

CC=g++
CXXFLAGS=-g
LIB=-lrt

default: all

all: radix radix.mpi

tests: test.cpp
	g++ test.cpp common.cpp -o $(EXE_TST) $(LIB)

radix: common.o radix.seq.o main.o
	g++ common.o radix.seq.o -o $(RAD_SEQ) $(LIB)

radix.mpi:
	mpic++ -g radix.mpi.cpp -c
	mpic++ -g common.o radix.mpi.o -o $(RAD_MPI) $(LIB)

clean:
	$(RM) *.o
