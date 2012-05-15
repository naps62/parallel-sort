#!/bin/bash
#
#PBS -V
#PBS -l nodes=16:r101:ppn=4
#PBS -l walltime=4:00:00
#PBS -N radix.mpi_64.bal
#PBS -m bea
#PBS -e out_101/mpi_64.bal.err
#PBS -o out_101/mpi_64.bal.out

cd $PBS_O_WORKDIR

NUM_EXECS=1
G=(4)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(64)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_101/mpi.bal
			output=results_101/mpi.bal/${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				mpirun -loadbalance -n $threads -machinefile $PBS_NODEFILE bin/radix.bal.mpi $size $g >> $output
			done
		done
	done
done
