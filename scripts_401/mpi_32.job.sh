#!/bin/bash
#
#PBS -V
#PBS -l nodes=4:r401:ppn=16
#PBS -l walltime=2:00:00
#PBS -N 401_radix.mpi_32
#PBS -m bea
#PBS -e out_401/mpi_32.err
#PBS -o out_401/mpi_32.out

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 4 8)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(32)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_401/mpi
			output=results_401/mpi/${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				mpirun -loadbalance -n $threads -machinefile $PBS_NODEFILE bin/radix.mpi $size $g >> $output
			done
		done
	done
done
