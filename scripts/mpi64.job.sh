#!/bin/bash
#
#PBS -l nodes=3:r601:ppn=24
#PBS -l walltime=2:00:00
#PBS -N radix.mpi64
#PBS -m bea
#PBS -e out/mpi.err
#PBS -o out/mpi.out

module load gnu/openmpi

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 4 8)
SIZES=(256 4096 65536 1048576 16777216)
THREADS=(64)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results/mpi
			output=results/mpi/${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				mpirun -n $threads -machinefile $PBS_NODEFILE bin/radix.mpi $size $g >> $output
			done
		done
	done
done
