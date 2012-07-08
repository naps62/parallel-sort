#!/bin/bash
#
#PBS -l nodes=1:hex:ppn=24
#PBS -l walltime=4:00:00
#PBS -N radix.tbb
#PBS -m bea
#PBS -e out_601/tbb.err
#PBS -o out_601/tbb.out

cd $PBS_O_WORKDIR

NUM_EXECS=5
G=(2 3 4 5 6 7 8)
SIZES=(2048 32768 524288 8388608 134217728)
THREADS=(4 8 16 32)

for g in ${G[@]}; do
	
	for size in ${SIZES[@]}; do

		for threads in ${THREADS[@]}; do

			mkdir -p results_601/tbb
			output=results_601/tbb/g${g}_s${size}_t${threads}
			rm -rf $output && touch $output

			for try in `seq 1 $NUM_EXECS`; do
				echo "running try $try, g=$g, size=$size, threads=$threads"
				echo "running try $try, g=$g, size=$size, threads=$threads" 1>&2
				bin/radix.omp $size $g $threads >> $output
			done
		done
	done
done
