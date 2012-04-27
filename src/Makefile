EXE_SEQ=../bin/sort.seq
EXE_OMP=../bin/sort.omp
EXE_MPI=../bin/sort.mpi
EXE_TST=../bin/tests

.PHONY: $(EXE_SEQ) $(EXE_OMP) $(EXE_MPI) $(EXE_TST)

LIB=-lrt

default: all

all: tests sequential omp mpi

tests: test.cpp
	g++ test.cpp -o $(EXE_TST) $(LIB)

sequential: $(EXE_SEQ)
	g++ sort.seq.cpp -o $(EXE_SEQ) $(LIB)

omp: $(EXE_OMP)
	@echo "OMP version: To be implemented"

mpi: $(EXE_MPI)
	@echo "MPI version: To be implemented"
