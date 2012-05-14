#!/bin/bash
#
#PBS -V
#PBS -l nodes=3:hex:ppn=24
#PBS -l walltime=2:00:00
#PBS -N 601_radix.mpi_64.bal
#PBS -m bea
#PBS -e out_601/mpi_64.bal.err
#PBS -o out_601/mpi_64.bal.out

cd $PBS_O_WORKDIR

NUM_EXECS=1
G=(2 4 8)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(64)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_601/mpi.bal
			output=results_601/mpi.bal/${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				mpirun -loadbalance -n $threads -machinefile $PBS_NODEFILE bin/radix.bal.mpi $size $g >> $output
			done
		done
	done
done
