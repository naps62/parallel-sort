#!/bin/bash
#
#PBS -V
#PBS -l nodes=1:r311:ppn=8
#PBS -l walltime=2:00:00
#PBS -N 311_radix.mpi_4_8
#PBS -m bea
#PBS -e out_311/mpi_4_8.err
#PBS -o out_311/mpi_4_8.out

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 4 8)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(4 8)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_311/mpi
			output=results_311/mpi/${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				mpirun -loadbalance -n $threads -machinefile $PBS_NODEFILE bin/radix.mpi $size $g >> $output
			done
		done
	done
done
