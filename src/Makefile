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

all: seq mpi rad rad.mpi

tests: test.cpp
	g++ test.cpp common.cpp -o $(EXE_TST) $(LIB)

seq: common.o sort.seq.o main.o
	g++ main.o common.o sort.seq.o -o $(EXE_SEQ) $(LIB)

mpi: $(EXE_MPI)
	@echo Not yet implemented
	#mpic++ sort.mpi.cpp -c
	#mpic++ main.o common.o sort.mpi.o -o $(EXE_MPI) $(LIB)

rad: common.o radix.seq.o main.o
	g++ main.o common.o radix.seq.o -o $(RAD_SEQ) $(LIB)

rad.mpi:
	mpic++ -g radix.mpi.cpp -c
	mpic++ -g main.o common.o radix.mpi.o -o $(RAD_MPI) $(LIB)

clean:
	$(RM) *.o
