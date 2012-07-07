#define _OMP
#include "common.shared_mem.cpp"

typedef unsigned int uint;
typedef vector<uint> radix_arr_t;

void radix_omp(radix_arr_t &arr, const unsigned int num_threads, const unsigned int g) {

	const unsigned int b	= (1 << g);				// num of buckets (2^g)
	unsigned int bpp		= b / num_threads;		// num of buckets per thread
	const unsigned int bpp_bc = popcount(bpp - 1);	// number of bits in bpp. used to compute BUCKET_TO_CPU



	vector<vector<unsigned int> > buckets(b);	// the buckets
	vector<unsigned int> bucket_accum(b);		// accumulated values for bucket_count. indicates where the values for each bucket should go
	bucket_accum[0] = 0;

	//
	// INITIALIZE THREADS
	//
	#pragma omp parallel num_threads(num_threads)
	{
		unsigned int thread_num = omp_get_thread_num();
		unsigned int mask = ((1 << g) - 1);		// initial mask to get key

		//
		// FOR EACH DIGIT
		//
		for(unsigned int round = 0; mask != 0; mask <<= g, ++round) {

			// clear buckets for this thread
			for(unsigned int i = 0; i < bpp; ++i) {
				buckets[thread_num * bpp + i].clear();
			}

			// fill buckets
			for(unsigned int i = 0; i < arr.size(); ++i) {
				unsigned int elem = arr[i];
				unsigned int bucket = GET_BUCKET_NUM(elem, mask, g, round);

				//if (round == 0)
				//	cout << elem << " goes to bucket " << bucket << " by thread " << BUCKET_TO_CPU(bucket) << endl;

				// if this bucket is handled by this thread, insert
				//if (round == 0) cout << "attempting " << BUCKET_TO_CPU(bucket) << " with " << thread_num << endl;
				if (BUCKET_TO_CPU(bucket) == thread_num) {
					//if (round == 0)
						//cout << "inserting " << elem << " into bucket " << bucket << endl;
					buckets[bucket].push_back(elem);
				}
			}

			#pragma omp barrier

			#pragma omp master
			{
				unsigned int sum = buckets[0].size();
				//cout << round << " bucket[0] = " << sum << endl;
				for(unsigned int i = 1; i < b; ++i) {
					bucket_accum[i] = sum;
					//cout << round << " bucket[" << i << "] = " << buckets[i].size() << endl;
					sum += buckets[i].size();
				}
			}

			#pragma omp barrier

			for(unsigned int i = 0; i < bpp; ++i) {
				unsigned int bucket = thread_num * bpp + i;
				unsigned int base_index = bucket_accum[bucket];

				for(unsigned int elem = 0; elem < buckets[bucket].size(); ++elem) {
					arr[base_index + elem] = buckets[bucket][elem];
				}
			}

			#pragma omp barrier
		}
	}

}